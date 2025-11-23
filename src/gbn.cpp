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
#include <vector>
using namespace std;

//defining variables
int message_buffer_size = 1000; 
float timeout = 20.0;
int base = 0;
int next_seqnum = 0;
int expected_seqnum = 0;
queue<struct msg> message_buffer;
vector<struct pkt> window;


// function to calculate the checksum
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

  while (next_seqnum < base + getwinsize() && !message_buffer.empty())
  { 
    struct msg message = message_buffer.front();
    message_buffer.pop();

    struct pkt packet;
    packet.seqnum = next_seqnum;
    packet.acknum = 0;
    strncpy(packet.payload, message.data, 20);
    packet.checksum = calculate_checksum(packet);

    if(window.size() < getwinsize())
    { 
      window.push_back(packet); 
    }
    else 
    {
      window[next_seqnum % getwinsize()] = packet;
     }

     tolayer3(0, packet);
     if(base == next_seqnum)
     {
      starttimer(0, timeout);
     }
    next_seqnum++;
  }
}

void A_input(struct pkt packet)
{
  if(calculate_checksum(packet) != packet.checksum || packet.acknum < base || packet.acknum >= next_seqnum)
  {
    return; 
  }
  base = packet.acknum + 1; 

  if (base == next_seqnum)
  { 
    stoptimer(0);
  }
  else
  { 
    starttimer(0, timeout); 
  }
  
  while (next_seqnum < base + getwinsize() && !message_buffer.empty())
  { 
    struct msg message = message_buffer.front();
    message_buffer.pop();
    struct pkt packet;
    packet.seqnum = next_seqnum;
    packet.acknum = 0;
    strncpy(packet.payload, message.data, 20);
    packet.checksum = calculate_checksum(packet);

    if(window.size() < getwinsize())
    { 
      window.push_back(packet); 
    }
    else 
    {
      window[next_seqnum % getwinsize()] = packet;
    }
     tolayer3(0, packet);
     if(base == next_seqnum)
     {
      starttimer(0, timeout);
     }
    next_seqnum++;
  }
}

void A_timerinterrupt()
{
  printf("Timeout. retransmit packets in current window");
  for (int i=base; i<next_seqnum; i++){ 
    tolayer3(0, window[i % getwinsize()]);
  }
  starttimer(0, timeout);
}  


void A_init()
{
  base = 0;
  next_seqnum = 0;
  window.clear();
  while(!message_buffer.empty())
  { 
    message_buffer.pop();
  }
}


void B_input(struct pkt packet)
{
  if(calculate_checksum(packet) != packet.checksum || packet.seqnum != expected_seqnum)
  { 
    struct pkt ACK_pkt;
    ACK_pkt.seqnum = 0;
    ACK_pkt.acknum = expected_seqnum - 1;
    ACK_pkt.checksum = calculate_checksum(ACK_pkt);
    tolayer3(1, ACK_pkt);
    return;
  }
  tolayer5(1, packet.payload);
  struct pkt ACK_pkt;
  ACK_pkt.seqnum = 0;
  ACK_pkt.acknum = expected_seqnum - 1;
  ACK_pkt.checksum = calculate_checksum(ACK_pkt);
  tolayer3(1, ACK_pkt);
  expected_seqnum++;
}

void B_init()
{
  expected_seqnum = 0;
}
