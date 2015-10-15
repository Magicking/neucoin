// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <csignal>
#include "wallet_ismine.h"

#include "key.h"
#include "keystore.h"
#include "script.h"

#include <boost/foreach.hpp>

using namespace std;

typedef vector<unsigned char> valtype;

unsigned int HaveKeys(const vector<valtype>& pubkeys, const CKeyStore& keystore)
{
    unsigned int nResult = 0;
    BOOST_FOREACH(const valtype& pubkey, pubkeys)
    {
        CKeyID keyID = CPubKey(pubkey).GetID();
        if (keystore.HaveKey(keyID))
            ++nResult;
    }
    return nResult;
}

template <typename T>
std::vector<unsigned char> ToByteVector(const T& in)
{
    return std::vector<unsigned char>(in.begin(), in.end());
}

namespace
{
    class CScriptVisitor : public boost::static_visitor<bool>
    {
    private:
        CScript *script;
    public:
        CScriptVisitor(CScript *scriptin) { script = scriptin; }

        bool operator()(const CNoDestination &dest) const {
            script->clear();
            return false;
        }

        bool operator()(const CKeyID &keyID) const {
            script->clear();
            *script << OP_DUP << OP_HASH160 << keyID << OP_EQUALVERIFY << OP_CHECKSIG;
            return true;
        }

        bool operator()(const CScriptID &scriptID) const {
            script->clear();
            *script << OP_HASH160 << scriptID << OP_EQUAL;
            return true;
        }
    };
}

CScript GetScriptForDestination(const CTxDestination& dest)
{
    CScript script;

    boost::apply_visitor(CScriptVisitor(&script), dest);
    return script;
}

isminetype IsMine(const CKeyStore &keystore, const CTxDestination& dest)
{
    CScript script = GetScriptForDestination(dest);
    return IsMine(keystore, script);
}

isminetype IsMine(const CKeyStore &keystore, const CScript& scriptPubKey)
{
    vector<valtype> vSolutions;
    txnouttype whichType;
    if (!Solver(scriptPubKey, whichType, vSolutions)) {
        if (keystore.HaveWatchOnly(scriptPubKey))
            return ISMINE_WATCH_ONLY;
        return ISMINE_NO;
    }

    CKeyID keyID;
    switch (whichType)
    {
    case TX_NONSTANDARD:
    case TX_NULL_DATA:
        break;
    case TX_PUBKEY:
        keyID = CPubKey(vSolutions[0]).GetID();
        if (keystore.HaveKey(keyID))
            return ISMINE_SPENDABLE;
        break;
    case TX_PUBKEYHASH:
        keyID = CKeyID(uint160(vSolutions[0]));
        if (keystore.HaveKey(keyID))
            return ISMINE_SPENDABLE;
        break;
    case TX_SCRIPTHASH:
    {
        CScriptID scriptID = CScriptID(uint160(vSolutions[0]));
        CScript subscript;
        if (keystore.GetCScript(scriptID, subscript))
            return(IsMine(keystore, subscript));
        break;
    }
    case TX_MULTISIG:
    {
        // Only consider transactions "mine" if we own ALL the
        // keys involved. Multi-signature transactions that are
        // partially owned (somebody else has a key that can spend
        // them) enable spend-out-from-under-you attacks, especially
        // in shared-wallet situations.
        vector<valtype> keys(vSolutions.begin()+1, vSolutions.begin()+vSolutions.size()-1);
        if (HaveKeys(keys, keystore) == keys.size())
            return ISMINE_SPENDABLE;
        break;
    }
    case TX_COLDMINTING:
    {
        CKeyID mintingKeyID = CKeyID(uint160(vSolutions[0]));
        CKeyID spendingKeyID = CKeyID(uint160(vSolutions[1]));
        if (keystore.HaveKey(mintingKeyID))
        {
            if (keystore.HaveKey(spendingKeyID))
                return ISMINE_ALL_SCRIPT;
            return ISMINE_WATCH_SOLVABLE;
        }
        else
            if (keystore.HaveKey(spendingKeyID))
                return ISMINE_SPENDABLE;
        break;
    }
    }

    return keystore.HaveWatchOnly(scriptPubKey) ? ISMINE_WATCH_ONLY : ISMINE_NO;
}
