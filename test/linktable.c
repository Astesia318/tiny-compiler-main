struct link_table {
	int val;
	struct link_table* next;
};

int main() {
	struct link_table lt[5];
	struct link_table* cur;
	int x;
	int i;
	for (i = 1; i < 5; i = i + 1) {
		lt[i].val = i;
		lt[i].next = &lt[i - 1];
	}
	cur = &lt[4];
	for (i = 0; i < 4; i = i + 1) {
		x = cur->val;
		cur = cur->next;
		output x;
	}
	return 0;
}