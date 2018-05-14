#pragma once

#include "common.h"
#include "async.h"
#include "../utility/bridge.h"
#include "../p2p/protocol.h"
#include "../p2p/connection.h"
#include "../utility/io/tcpserver.h"

namespace beam {
namespace proto {

#define BeamNodeMsg_NewTip(macro) \
	macro(Block::SystemState::ID, ID)

#define BeamNodeMsg_GetHdr(macro) \
	macro(Block::SystemState::ID, ID)

#define BeamNodeMsg_Hdr(macro) \
	macro(Block::SystemState::Full, Description)

#define BeamNodeMsg_DataMissing(macro)

#define BeamNodeMsg_Boolean(macro) \
	macro(bool, Value)

#define BeamNodeMsg_GetBody(macro) \
	macro(Block::SystemState::ID, ID)

#define BeamNodeMsg_Body(macro) \
	macro(ByteBuffer, Buffer)

#define BeamNodeMsg_GetProofState(macro) \
	macro(Block::SystemState::ID, ID)

#define BeamNodeMsg_GetProofKernel(macro) \
	macro(Merkle::Hash, KernelHash)

#define BeamNodeMsg_GetProofUtxo(macro) \
	macro(Input, Utxo) \
	macro(Height, MaturityMin) /* set to non-zero in case the result is too big, and should be retrieved within multiple queries */

	 
#define BeamNodeMsg_Proof(macro) \
	macro(Block::SystemState::ID, ID) \
	macro(Merkle::Proof, Proof)

#define BeamNodeMsg_ProofUtxo(macro) \
	macro(Block::SystemState::ID, ID) \
	macro(std::vector<PerUtxoProof>, Proofs)

#define BeamNodeMsg_Ping(macro)
#define BeamNodeMsg_Pong(macro)


#define BeamNodeMsgsAll(macro) \
	macro(1, NewTip) /* Also the first message sent by the node */ \
	macro(2, GetHdr) \
	macro(3, Hdr) \
	macro(4, DataMissing) \
	macro(5, Boolean) \
	macro(6, GetBody) \
	macro(7, Body) \
	macro(8, GetProofState) \
	macro(9, GetProofKernel) \
	macro(10, GetProofUtxo) \
	macro(11, Proof) /* for states and kernels */ \
	macro(12, ProofUtxo) \
	macro(21, Ping) \
	macro(22, Pong)


	struct PerUtxoProof
	{
		Height m_Maturity;
		Input::Count m_Count;
		Merkle::Proof m_Proof;

		template <typename Archive>
		void serialize(Archive& ar)
		{
			ar
				& m_Maturity
				& m_Count
				& m_Proof;
		}

		static const uint32_t s_EntriesMax = 20; // if this is the size of the vector - the result is probably trunacted
	};


#define THE_MACRO3(type, name) & m_##name
#define THE_MACRO2(type, name) type m_##name;
#define THE_MACRO1(code, msg) \
	struct msg \
	{ \
		BeamNodeMsg_##msg(THE_MACRO2) \
 		template <typename Archive> void serialize(Archive& ar) { ar BeamNodeMsg_##msg(THE_MACRO3); } \
	};

	BeamNodeMsgsAll(THE_MACRO1)
#undef THE_MACRO1
#undef THE_MACRO2
#undef THE_MACRO3



	class NodeConnection
		:public IMsgHandler
	{
		Protocol<NodeConnection> m_Protocol;
		std::unique_ptr<Connection> m_Connection;
		bool m_ConnectPending;

		SerializedMsg m_SerializeCache;

		static void TestIoResult(const io::Result& res);

		static void OnConnectInternal(uint64_t tag, io::TcpStream::Ptr&& newStream, int status);
		void OnConnectInternal2(io::TcpStream::Ptr&& newStream, int status);

		virtual void on_protocol_error(uint64_t, ProtocolError error) override;
		virtual void on_connection_error(uint64_t, int errorCode) override;

#define THE_MACRO(code, msg) bool OnMsgInternal(uint64_t, msg&& v);
		BeamNodeMsgsAll(THE_MACRO)
#undef THE_MACRO

	public:

		NodeConnection();
		virtual ~NodeConnection();
		void Reset();

		void Connect(const io::Address& addr);
		void Accept(io::TcpStream::Ptr&& newStream);

		virtual void OnConnected() {}
		virtual void OnClosed(int errorCode) {}

#define THE_MACRO(code, msg) \
		void Send(const msg& v); \
		virtual void OnMsg(msg&& v) {}
		BeamNodeMsgsAll(THE_MACRO)
#undef THE_MACRO

		struct Server
		{
			io::TcpServer::Ptr m_pServer; // just delete it to stop listening
			void Listen(const io::Address& addr);

			virtual void OnAccepted(io::TcpStream::Ptr&&, int errorCode) = 0;
		};
	};


} // namespace proto
} // namespace beam
