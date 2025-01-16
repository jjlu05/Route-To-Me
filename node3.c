#include <stdio.h>
#include "project3.h"
#include <stdbool.h>
extern int TraceLevel;
extern float clocktime;

struct distance_table {
  int costs[MAX_NODES][MAX_NODES];
};
struct distance_table dt3;
struct NeighborCosts   *neighbor3;
int listOfNeighbors3[MAX_NODES]; 
int numNeighbors3 = 0;



void rtinit3() {
     for (int i = 0; i < MAX_NODES; i++) {
        if (i == 3) {
            dt3.costs[3][i] = 0;   
        } else {
             dt3.costs[3][i] = getNeighborCosts(3)->NodeCosts[i];
        }
    }

      printf("At time t=%f, rtinit3() called.\n");
    printf("At time t=%f, node 3 initial distance vector: ");
    for (int i = 0; i < MAX_NODES; i++) {
        printf("%d ", dt3.costs[3][i]);
    }
    printf("\n");

     for (int i = 0; i < MAX_NODES; i++) {
         if (i != 3 && getNeighborCosts(3)->NodeCosts[i] != INFINITY) {
            struct RoutePacket pkt;
            pkt.sourceid = 3;
            pkt.destid = i;

             for (int j = 0; j < MAX_NODES; j++) {
                pkt.mincost[j] = dt3.costs[3][j];
            }

             printf("At time t=%f, node 3 sends packet to node %d with: ", clocktime,i);
            for (int j = 0; j < MAX_NODES; j++) {
                printf("%d ", pkt.mincost[j]);
            }
            printf("\n");

             toLayer2(pkt);
        }
    }
 for (int i = 0; i < MAX_NODES; i++) {
    
if (dt3.costs[3][i] != INFINITY && dt3.costs[3][i] != 0) {
        listOfNeighbors3[numNeighbors3] = i;  
        numNeighbors3++;
}
}
}




void rtupdate3(struct RoutePacket *rcvdpkt) {
       int listOfNeighbors3[MAX_NODES];  
int numNeighbors3 = 0;
printf("At time t=%f, rtupdate3() called, by a pkt received from Sender id: %d.\n",clocktime, rcvdpkt->sourceid);
   bool changed = false;

for (int i = 0; i < MAX_NODES; i++) {
    
if (dt3.costs[3][i] != INFINITY && dt3.costs[3][i] != 0) {
        listOfNeighbors3[numNeighbors3] = i; 
        numNeighbors3++;
}
     int calcVal = rcvdpkt->mincost[i] + dt3.costs[3][rcvdpkt->sourceid];

     if (calcVal < dt3.costs[3][i]) {
        dt3.costs[3][i] = calcVal;
        changed = true;
    }
}

if (changed) {
     struct RoutePacket testPacket;
    testPacket.sourceid = 3;

    for (int j = 0; j < MAX_NODES; j++) {
        testPacket.mincost[j] = dt3.costs[3][j];
    }
printf("At time t=%f, node 3 current distance vector: ",clocktime);

        for (int j = 0; j < MAX_NODES; j++) {
            printf("%d ", testPacket.mincost[j]);
        }
                printf("\n");

     for (int i = 0; i < numNeighbors3; i++) {
        testPacket.destid = listOfNeighbors3[i];
        toLayer2(testPacket);

        printf("At time t=%f, node 3 sends packet to node %d with: ", clocktime, testPacket.destid);
        for (int j = 0; j < MAX_NODES; j++) {
            printf("%d ", testPacket.mincost[j]);
        }
        printf("\n");
    }
}
}
/////////////////////////////////////////////////////////////////////
//  printdt
//  This routine is being supplied to you.  It is the same code in
//  each node and is tailored based on the input arguments.
//  Required arguments:
//  MyNodeNumber:  This routine assumes that you know your node
//                 number and supply it when making this call.
//  struct NeighborCosts *neighbor:  A pointer to the structure 
//                 that's supplied via a call to getNeighborCosts().
//                 It tells this print routine the configuration
//                 of nodes surrounding the node we're working on.
//  struct distance_table *dtptr: This is the running record of the
//                 current costs as seen by this node.  It is 
//                 constantly updated as the node gets new
//                 messages from other nodes.
/////////////////////////////////////////////////////////////////////
void printdt3( int MyNodeNumber, struct NeighborCosts *neighbor, 
		struct distance_table *dtptr ) {
    int       i, j;
    int       TotalNodes = neighbor->NodesInNetwork;     // Total nodes in network
    int       NumberOfNeighbors = 0;                     // How many neighbors
    int       Neighbors[MAX_NODES];                      // Who are the neighbors

    // Determine our neighbors 
    for ( i = 0; i < TotalNodes; i++ )  {
        if (( neighbor->NodeCosts[i] != INFINITY ) && i != MyNodeNumber )  {
            Neighbors[NumberOfNeighbors] = i;
            NumberOfNeighbors++;
        }
    }
    // Print the header
    printf("                via     \n");
    printf("   D%d |", MyNodeNumber );
    for ( i = 0; i < NumberOfNeighbors; i++ )
        printf("     %d", Neighbors[i]);
    printf("\n");
    printf("  ----|-------------------------------\n");

    // For each node, print the cost by travelling thru each of our neighbors
    for ( i = 0; i < TotalNodes; i++ )   {
        if ( i != MyNodeNumber )  {
            printf("dest %d|", i );
            for ( j = 0; j < NumberOfNeighbors; j++ )  {
                    printf( "  %4d", dtptr->costs[i][Neighbors[j]] );
            }
            printf("\n");
        }
    }
    printf("\n");
}    // End of printdt3

