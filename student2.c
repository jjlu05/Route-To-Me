#include <stdio.h>
#include <stdlib.h>
#include "project2.h"
#include <string.h>




struct msgQueue {
    struct msg message;
    struct msgQueue *next;
};
struct msgQueue *head = NULL;
struct msgQueue *tail = NULL;
int test = 0;


void enqueue(struct msg message) {
    struct msgQueue *newNode = (struct msgQueue *)malloc(sizeof(struct msgQueue));
    newNode->message= message;
    newNode->next = NULL;
    if (tail == NULL) {
        head=tail=newNode;
    } else {
        tail->next=newNode;
        tail= newNode;
    }
}






struct msg dequeue() {
    if (head == NULL) {
        struct msg emptyMsg = { .data = {0} };
        return emptyMsg; 
    }
    struct msgQueue *temp = head;
    struct msg message = temp->message;
    head = head->next;
    if (head == NULL) {
        tail = NULL;
    }
    free(temp);
    return message;
}
int EmptyQ() {
    return head == NULL;
}




static int testASN = 0;
static int expectedSNB = 0;
static struct pkt lastPacket;
static int waitForACK = 0;

//decided to use packet parameter rather than 3 parameters for ease
//my checksum for some reason before wasn't working well enough, or was inconsistent so I had to give it more variability
int calculateChecksum(struct pkt packet) {
    int checksum = packet.seqnum + packet.acknum;
    for (int i = 0; i < MESSAGE_LENGTH; i++) {
        checksum = (checksum << 1) + packet.payload[i];
    }
    return ~checksum; 
}







/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */

void A_output(struct msg message) {
    if (waitForACK) {
        enqueue(message);
        return;
    }
    struct pkt packet;
    strncpy(packet.payload,message.data,MESSAGE_LENGTH);   
    packet.seqnum = testASN;
    packet.acknum = 0;
    packet.checksum = calculateChecksum(packet);

    tolayer3(AEntity, packet);

    startTimer(AEntity, 20.0);
    lastPacket = packet;
    waitForACK =1;

}




/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)  {
printf("Hello!-B_output");
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {
    int checksum = calculateChecksum(packet);
    if (checksum != packet.checksum) {
      test = 1;
      return;
    }

    if (packet.acknum == testASN) {
        stopTimer(AEntity);
        testASN = 1-testASN;
        waitForACK = 0;
        if (!EmptyQ()) {
          struct msg nextMessage = dequeue();
          A_output(nextMessage);
        }
    } 
    else {

    }
}



/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
    tolayer3(AEntity, lastPacket);
    startTimer(AEntity, 20.0);
}


/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
    testASN = 0;
    waitForACK = 0;
}



/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
    int checksum = calculateChecksum(packet);
    if (checksum != packet.checksum) {
        return;
    }

    if (packet.seqnum == expectedSNB) {
            expectedSNB = 1 - expectedSNB; 
        struct msg receivedMessage;
        strncpy(receivedMessage.data, packet.payload, MESSAGE_LENGTH);


        tolayer5(BEntity, receivedMessage); 
    } 

    struct pkt ACKPacket;
    ACKPacket.seqnum = 0;
    ACKPacket.acknum = packet.seqnum;
    ACKPacket.checksum = calculateChecksum(ACKPacket);

    tolayer3(BEntity, ACKPacket);
}
/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void B_timerinterrupt() {
  printf("GOT CALLED!");
}


/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
    expectedSNB = 0;
}


