import { compileWith }                                             from './framework/compilation';
import { mineSomePowBlocks, mintSomePosBlocks, mineForMaturation } from './framework/mining';
import { sendRpcQuery }                                            from './framework/query';
import { spawnClient }                                             from './framework/spawn';
import { delayExecution }                                          from './framework/time';

import { fastChain, smallChain }                                   from './_environments';

export async function test( ) {

    await compileWith( fastChain, smallChain );

    var client1 = await spawnClient( );
    var client2 = await spawnClient( { addnode : client1.target } );
    var client3 = await spawnClient( { addnode : client2.target } );

    await mineSomePowBlocks( client1, 1 );
    await delayExecution( 5 );
    await mineSomePowBlocks( client3, 64 );
    await delayExecution( 10 );

    var rpc = await sendRpcQuery( client1, { method : 'getbalance' } );

    expect( rpc.result ).to.equal( 1 );

    var { result : spendingAddress } = await sendRpcQuery( client1, { method : 'getnewaddress' } );
    var { result : mintingAddress } = await sendRpcQuery( client1, { method : 'getnewaddress' } );

    var { result : mintingPrivateKey } = await sendRpcQuery( client1, { method : 'dumpprivkey', params : [ mintingAddress ] } );
    await sendRpcQuery( client2, { method : 'importprivkey', params : [ mintingPrivateKey ] } );

    var { result : coldMintingAddress1 } = await sendRpcQuery( client1, { method : 'addcoldmintingaddress', params : [ mintingAddress, spendingAddress ] } );

    var rpc = await sendRpcQuery( client1, { method : 'sendtoaddress', params : [ coldMintingAddress1, 1 ] } );

    await delayExecution( 2 );

    await mineSomePowBlocks( client3, 1 );

    await delayExecution( 2 );

    var rpc = await sendRpcQuery( client1, { method : 'getbalance' } );

    expect( rpc.result ).to.equal( 1 );

    var { result : coldMintingAddress2 } = await sendRpcQuery( client2, { method : 'addcoldmintingaddress', params : [ mintingAddress, spendingAddress ] } );

    expect( coldMintingAddress1 ).to.equal( coldMintingAddress2 );

    await sendRpcQuery( client2, { method : 'ignoreconnection', params : [ true ] } );
    await sendRpcQuery( client2, { method : 'addnode', params : [ client1.target, 'remove' ] } );
    await mintSomePosBlocks( client3, 1 );
    await mineForMaturation( client3, 1 );
    await mintSomePosBlocks( client3, 1 );

    await mintSomePosBlocks( client2, 1 );

    var { result : head1 } = await sendRpcQuery( client1, { method : 'getbestblockhash' } );
    var { result : head2 } = await sendRpcQuery( client2, { method : 'getbestblockhash' } );

    expect( head1 ).not.to.equal( head2 );

    await sendRpcQuery( client2, { method : 'ignoreconnection', params : [ false ] } );
    await sendRpcQuery( client2, { method : 'addnode', params : [ client3.target, 'onetry' ] } );

//  Force reorganization by sending block
    await mineForMaturation( client3, 1 );

    await delayExecution( 10 );

    var { result : head1 } = await sendRpcQuery( client1, { method : 'getbestblockhash' } );
    var { result : head2 } = await sendRpcQuery( client2, { method : 'getbestblockhash' } );
    expect( head1 ).to.equal( head2 );

    var { result : listunspent } = await sendRpcQuery( client2, { method : 'listunspent', params : [ 0, 99999, [] , true ] } );
    expect( listunspent.length ).to.equal( 1 );

    await mintSomePosBlocks( client2, 1 );
    await delayExecution( 5 );
    await mineForMaturation( client3, 1 );

    await delayExecution( 1 );
    var rpc = await sendRpcQuery( client1, { method : 'getbalance' } );
    expect( rpc.result ).to.equal( 2 );
}
