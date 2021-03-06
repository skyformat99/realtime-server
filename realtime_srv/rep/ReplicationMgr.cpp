#include "realtime_srv/common/RealtimeSrvShared.h"



void ReplicationMgr::ReplicateCreate( int inNetworkId, uint32_t inInitialDirtyState )
{
	mNetworkIdToReplicationCommand[inNetworkId] = ReplicationCmd( inInitialDirtyState );
}

void ReplicationMgr::ReplicateDestroy( int inNetworkId )
{
	mNetworkIdToReplicationCommand[inNetworkId].SetDestroy();
}

void ReplicationMgr::RemoveFromReplication( int inNetworkId )
{
	mNetworkIdToReplicationCommand.erase( inNetworkId );
}

void ReplicationMgr::SetReplicationStateDirty( int inNetworkId, uint32_t inDirtyState )
{
	mNetworkIdToReplicationCommand[inNetworkId].AddDirtyState( inDirtyState );
}

void ReplicationMgr::HandleCreateAckd( int inNetworkId )
{
	mNetworkIdToReplicationCommand[inNetworkId].HandleCreateAckd();
}


void ReplicationMgr::Write( OutputBitStream& inOutputStream, InFlightPacket* inInFlightPacket )
{
	for (auto& pair : mNetworkIdToReplicationCommand)
	{
		ReplicationCmd& replicationCommand = pair.second;
		if (replicationCommand.HasDirtyState())
		{
			int networkId = pair.first;
			
			inOutputStream.Write( networkId );
			
			ReplicationAction action = replicationCommand.GetAction();
			inOutputStream.Write( action, 2 );

			uint32_t writtenState = 0;
			uint32_t dirtyState = replicationCommand.GetDirtyState();
			
			switch (action)
			{
			case RA_Create:
				writtenState = WriteCreateAction( inOutputStream,
					networkId, dirtyState );
				break;
			case RA_Update:
				writtenState = WriteUpdateAction( inOutputStream,
					networkId, dirtyState );
				break;
			case RA_Destroy:
				writtenState = WriteDestroyAction( inOutputStream,
					networkId, dirtyState );
				break;
			default:
				break;
			}

			inInFlightPacket->AddTransmission( networkId, action, writtenState );

			replicationCommand.ClearDirtyState( writtenState );

			if (inOutputStream.GetByteLength() > MAX_PACKET_BYTE_LENGTH )
			{
				break;
			}
		}
	}
}


uint32_t ReplicationMgr::WriteCreateAction( OutputBitStream& inOutputStream, 
	int inNetworkId, uint32_t inDirtyState )
{
	GameObjPtr gameObject = owner_->GetWorld()->GetGameObject( inNetworkId );
	
	inOutputStream.Write( gameObject->GetClassId() );
	return gameObject->Write( inOutputStream, inDirtyState );
}

uint32_t ReplicationMgr::WriteUpdateAction( OutputBitStream& inOutputStream, 
	int inNetworkId, uint32_t inDirtyState )
{
	GameObjPtr gameObject = owner_->GetWorld()->GetGameObject( inNetworkId );

	uint32_t writtenState = gameObject->Write( inOutputStream, inDirtyState );

	return writtenState;
}

uint32_t ReplicationMgr::WriteDestroyAction( OutputBitStream& inOutputStream, 
	int inNetworkId, uint32_t inDirtyState )
{
	( void )inOutputStream;
	( void )inNetworkId;
	( void )inDirtyState;
	

	return inDirtyState;
}