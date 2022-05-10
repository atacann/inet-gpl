//
// Copyright (C) 2004-2010 OpenSim Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
//

#ifndef __INETGPL_TCPNSCQUEUES_H
#define __INETGPL_TCPNSCQUEUES_H

#include "inet/common/packet/ChunkQueue.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inetgpl/transportlayer/tcp_nsc/TcpNscConnection.h"

namespace inetgpl {

namespace tcp {

/**
 * Send queue that manages actual bytes.
 */
class INETGPL_API TcpNscSendQueue : public cObject
{
  public:
    /**
     * Ctor.
     */
    TcpNscSendQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpNscSendQueue();

    /**
     * set connection queue, and initialise queue variables.
     */
    virtual void setConnection(TcpNscConnection *connP);

    /**
     * Called on SEND app command, it inserts in the queue the data the user
     * wants to send. Implementations of this abstract class will decide
     * what this means: copying actual bytes, just increasing the
     * "last byte queued" variable, or storing cMessage object(s).
     * The msg object should not be referenced after this point (sendQueue may
     * delete it.)
     */
    virtual void enqueueAppData(Packet *msg);

    /**
     * Copy data to the buffer for send to NSC.
     * returns lengh of copied data.
     * create msg for socket->send_data()
     *
     * called before called socket->send_data()
     */
    virtual int getBytesForTcpLayer(void *bufferP, int bufferLengthP) const;

    /**
     * The function should remove msgLengthP bytes from NSCqueue
     *
     * But the NSC sometimes reread from this datapart (when data destroyed in IP Layer)
     * inside of createSegmentWithBytes() function.
     *
     * called with return value of socket->send_data() if larger than 0
     */
    virtual void dequeueTcpLayerMsg(int msgLengthP);

    /**
     * Utility function: returns how many bytes are available in the queue.
     */
    virtual unsigned long getBytesAvailable() const;

    /**
     * Called when the TCP wants to send or retransmit data, it constructs
     * a TCP segment which contains the data from the requested sequence
     * number range. The actually returned segment may contain less then
     * maxNumBytes bytes if the subclass wants to reproduce the original
     * segment boundaries when retransmitting.
     *
     * called from inside of send_callback()
     * called before called the send() to IP layer
     */
    virtual Packet *createSegmentWithBytes(const void *tcpDataP, int tcpLengthP);

    /**
     * Tells the queue that bytes up to (but NOT including) seqNum have been
     * transmitted and ACKed, so they can be removed from the queue.
     */
    virtual void discardUpTo(uint32_t seqNumP);

  protected:
    TcpNscConnection *connM = nullptr;
    ChunkQueue dataBuffer;
};

/**
 * Receive queue that manages actual bytes.
 */
class INETGPL_API TcpNscReceiveQueue : public cObject
{
  public:
    /**
     * Ctor.
     */
    TcpNscReceiveQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpNscReceiveQueue();

    /**
     * Add a connection queue.
     */
    virtual void setConnection(TcpNscConnection *connP);

    /**
     * Called when a TCP segment arrives. The segment
     * object should *not* be deleted.
     *
     * For remove and store payload messages if need.
     *
     * called before nsc_stack->if_receive_packet() called
     */
    virtual void notifyAboutIncomingSegmentProcessing(Packet *packet);

    /**
     * The method called when data received from NSC
     * The method should set status of the data in queue to received
     * called after socket->read_data() successfull
     */
    virtual void enqueueNscData(void *dataP, int dataLengthP);

    /**
     * Should create a packet to be passed up to the app, up to (but NOT
     * including) the given sequence no (usually rcv_nxt).
     * It should return nullptr if there's no more data to be passed up --
     * this method is called several times until it returns nullptr.
     *
     * called after socket->read_data() successfull
     */
    virtual Packet *extractBytesUpTo();

    /**
     * Returns the number of bytes (out-of-order-segments) currently buffered in queue.
     */
    virtual uint32_t getAmountOfBufferedBytes() const;

    /**
     * Returns the number of blocks currently buffered in queue.
     */
    virtual uint32_t getQueueLength() const;

    /**     * Shows current queue status.
     */
    virtual void getQueueStatus() const;

    /**
     * notify the queue about output messages
     *
     * called when connM send out a packet.
     * for read AckNo, if have
     */
    virtual void notifyAboutSending(const Packet *packet);

  protected:
    ChunkQueue dataBuffer; // fifo dataBuffer
    TcpNscConnection *connM = nullptr;
};

} // namespace tcp

} // namespace inet

#endif

