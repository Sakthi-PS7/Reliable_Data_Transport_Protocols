#include "../include/simulator.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES ********  */

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <queue>
using namespace std;

//defining variables
int message_buffer_size = 1000;
float timeout = 10.0;
int sequence_number = 0;
int expected_seqnum = 0;
bool expecting_ACK = false;
struct pkt last_pkt;
queue<struct msg> message_buffer;


// function to calculate checksum
int calculate_checksum(const struct pkt &packet)
{
    int checksum = packet.seqnum + packet.acknum;
    for (int i = 0; i < 20; i++)
    {
        checksum += packet.payload[i];
    }
    return checksum;
}

void A_output(struct msg message)
{
  if (message_buffer.size() >= message_buffer_size)
  {
    printf("Buffer exceeded");
    return;
  }
  message_buffer.push(message);
  if(expecting_ACK )
  {
    return;
  }
  message = message_buffer.front();
  message_buffer.pop();
  struct pkt packet;
  packet.seqnum = sequence_number;
  packet.acknum = 0;
  strncpy(packet.payload, message.data, 20);
  packet.checksum = calculate_checksum(packet);
  last_pkt = packet;
  tolayer3(0, last_pkt);
  starttimer(0, timeout);
  expecting_ACK = true;
}


void A_input(struct pkt packet)
{
    if (packet.checksum != calculate_checksum(packet) || packet.acknum != sequence_number)
    {
        return;
    }
    stoptimer(0);
    sequence_number = 1 - sequence_number;
    expecting_ACK = false;

    if(expecting_ACK || message_buffer.empty())
    {
        return;
    }

    struct msg message = message_buffer.front();
    message_buffer.pop();
    struct pkt new_packet;
    new_packet.seqnum = sequence_number;
    new_packet.acknum = 0;
    strncpy(new_packet.payload, message.data, 20);
    new_packet.checksum = calculate_checksum(new_packet);
    last_pkt = new_packet;
    tolayer3(0, last_pkt);
    starttimer(0, timeout);
    expecting_ACK = true;
}

void A_timerinterrupt()
{
    tolayer3(0, last_pkt);
    starttimer(0, timeout);
}

void A_init()
{
    sequence_number = 0;
    expecting_ACK = false;

    while(!message_buffer.empty())
    {
        message_buffer.pop();
    }
}
void B_input(struct pkt packet)
{
    if (packet.checksum != calculate_checksum(packet) || packet.seqnum != expected_seqnum){
        struct pkt ACK_pkt;
        ACK_pkt.seqnum = 0;
        ACK_pkt.acknum = 1 - expected_seqnum;
        ACK_pkt.checksum = calculate_checksum(ACK_pkt);
        tolayer3(1, ACK_pkt);
        return;
    }
    tolayer5(1, packet.payload);

    struct pkt ACK_pkt;
    ACK_pkt.seqnum = 0;
    ACK_pkt.acknum = expected_seqnum;
    ACK_pkt.checksum = calculate_checksum(ACK_pkt);
    tolayer3(1, ACK_pkt);
    expected_seqnum = 1 - expected_seqnum;
}
void B_init()
{
   expected_seqnum = 0;
}
