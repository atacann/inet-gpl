//
// Copyright (C) 2004-2010 OpenSim Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
//

#include "inetgpl/common/INETGPLDefs.h"
#include "inetgpl/transportlayer/tcp_nsc/queues/TcpNscQueues.h"
#include "inetgpl/transportlayer/tcp_nsc/TcpNscConnection.h"

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inetgpl {

namespace tcp {
using namespace inet::tcp;

Register_Class(TcpNscSendQueue);

Register_Class(TcpNscReceiveQueue);

TcpNscSendQueue::TcpNscSendQueue()
{
}

TcpNscSendQueue::~TcpNscSendQueue()
{
}

void TcpNscSendQueue::setConnection(TcpNscConnection *connP)
{
    dataBuffer.clear();
    connM = connP;
}

void TcpNscSendQueue::enqueueAppData(Packet *msg)
{
    ASSERT(msg);
    dataBuffer.push(msg->peekDataAt(B(0), B(msg->getByteLength())));
    delete msg;
}

int TcpNscSendQueue::getBytesForTcpLayer(void *bufferP, int bufferLengthP) const
{
    ASSERT(bufferP);

    int length = B(dataBuffer.getLength()).get();
    if (bufferLengthP < length)
        length = bufferLengthP;
    if (length == 0)
        return 0;

    const auto& bytesChunk = dataBuffer.peek<BytesChunk>(B(length));
    return bytesChunk->copyToBuffer(static_cast<uint8_t *>(bufferP), length);
}

void TcpNscSendQueue::dequeueTcpLayerMsg(int msgLengthP)
{
    dataBuffer.pop(B(msgLengthP));
}

ulong TcpNscSendQueue::getBytesAvailable() const
{
    return B(dataBuffer.getLength()).get();
}

Packet *TcpNscSendQueue::createSegmentWithBytes(const void *tcpDataP, int tcpLengthP)
{
    ASSERT(tcpDataP);

    const auto& bytes = makeShared<BytesChunk>(static_cast<const uint8_t *>(tcpDataP), tcpLengthP);
    auto packet = new Packet(nullptr, bytes);
    const auto& tcpHdr = packet->popAtFront<TcpHeader>();
    packet->trimFront();
    int64_t numBytes = packet->getByteLength();
    packet->insertAtFront(tcpHdr);

//    auto payload = makeShared<BytesChunk>((const uint8_t*)tcpDataP, tcpLengthP);
//    const auto& tcpHdr = (payload->Chunk::peek<TcpHeader>(byte(0)));
//    payload->removeFromBeginning(tcpHdr->getChunkLength());

    char msgname[80];
    sprintf(msgname, "%.10s%s%s%s(l=%lu bytes)",
            "tcpHdr",
            tcpHdr->getSynBit() ? " SYN" : "",
            tcpHdr->getFinBit() ? " FIN" : "",
            (tcpHdr->getAckBit() && 0 == numBytes) ? " ACK" : "",
            (unsigned long)numBytes);

    return packet;
}

void TcpNscSendQueue::discardUpTo(uint32_t seqNumP)
{
    // nothing to do here
}

////////////////////////////////////////////////////////////////////////////////////////

TcpNscReceiveQueue::TcpNscReceiveQueue()
{
}

TcpNscReceiveQueue::~TcpNscReceiveQueue()
{
    // nothing to do here
}

void TcpNscReceiveQueue::setConnection(TcpNscConnection *connP)
{
    ASSERT(connP);

    dataBuffer.clear();
    connM = connP;
}

void TcpNscReceiveQueue::notifyAboutIncomingSegmentProcessing(Packet *packet)
{
    ASSERT(packet);
}

void TcpNscReceiveQueue::enqueueNscData(void *dataP, int dataLengthP)
{
    dataBuffer.push(makeShared<BytesChunk>(static_cast<uint8_t *>(dataP), dataLengthP));
}

Packet *TcpNscReceiveQueue::extractBytesUpTo()
{
    ASSERT(connM);

    Packet *dataMsg = nullptr;
    b queueLength = dataBuffer.getLength();

    if (queueLength > b(0)) {
        dataMsg = new Packet("DATA", TCP_I_DATA);
        const auto& data = dataBuffer.pop<Chunk>(queueLength);
        dataMsg->insertAtBack(data);
    }

    return dataMsg;
}

uint32_t TcpNscReceiveQueue::getAmountOfBufferedBytes() const
{
    return B(dataBuffer.getLength()).get();
}

uint32_t TcpNscReceiveQueue::getQueueLength() const
{
    return B(dataBuffer.getLength()).get();
}

void TcpNscReceiveQueue::getQueueStatus() const
{
    // TODO
}

void TcpNscReceiveQueue::notifyAboutSending(const Packet *packet)
{
    // nothing to do
}

} // namespace tcp

} // namespace inet

