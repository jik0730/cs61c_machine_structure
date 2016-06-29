#include <stdio.h>

typedef struct node {
	int value;
	struct node *next;
} node;

int ll_has_cycle(node *head) {
	/* your code here */
	node *tortoise = head;
	node *hare = head;

	if(head == NULL) {
		return 0;
	}
	
	while(1) {
		if(hare->next != NULL && hare->next->next != NULL) {
		hare = hare->next->next;
		tortoise = tortoise->next;
		if(hare == tortoise) {
			return 1;
		}
		} else {
			return 0;
		}
	}
}

void test_ll_has_cycle(void) {
	int i;
	node nodes[25]; //enough to run our tests
	for(i=0; i < sizeof(nodes)/sizeof(node); i++) {
		nodes[i].next = 0;
		nodes[i].value = 0;
	}
	nodes[0].next = &nodes[1];
	nodes[1].next = &nodes[2];
	nodes[2].next = &nodes[3];
	printf("First list has no cycle, ll_has_cycle says it has %s cycle\n", ll_has_cycle(&nodes[0])?"a":"no");
  
	nodes[4].next = &nodes[5];
	nodes[5].next = &nodes[6];
	nodes[6].next = &nodes[7];
	nodes[7].next = &nodes[8];
	nodes[8].next = &nodes[9];
	nodes[9].next = &nodes[10];
	nodes[10].next = &nodes[4];
	printf("Second list has a cycle, ll_has_cycle says it has %s cycle\n", ll_has_cycle(&nodes[4])?"a":"no");
  
	nodes[11].next = &nodes[12];
	nodes[12].next = &nodes[13];
	nodes[13].next = &nodes[14];
	nodes[14].next = &nodes[15];
	nodes[15].next = &nodes[16];
	nodes[16].next = &nodes[17];
	nodes[17].next = &nodes[14];
	printf("Third list has a cycle, ll_has_cycle says it has %s cycle\n", ll_has_cycle(&nodes[11])?"a":"no");
  
	nodes[18].next = &nodes[18];
	printf("Fourth list has a cycle, ll_has_cycle says it has %s cycle\n", ll_has_cycle(&nodes[18])?"a":"no");
  
	nodes[19].next = &nodes[20];
	nodes[20].next = &nodes[21];
	nodes[21].next = &nodes[22];
	nodes[22].next = &nodes[23];
	printf("Fifth list has no cycle, ll_has_cycle says it has %s cycle\n", ll_has_cycle(&nodes[19])?"a":"no");
  
	printf("Length-zero list has no cycle, ll_has_cycle says it has %s cycle\n", ll_has_cycle(NULL)?"a":"no");
}

int main(void) {
  test_ll_has_cycle();
  return 0;
}

