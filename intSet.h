struct IntElement{
	int value;
	struct IntElement* next;
};

struct IntSet{
	struct IntElement* first;
	struct IntElement* last;
	size_t size;
};

struct IntElement* getIntElement();
struct IntSet* getIntSet();
void dumpIntElements(struct IntElement* e);
void printIntSet(struct IntSet* s, FILE*out);
void dumpIntSet(struct IntSet* s);
void appendInt(struct IntSet* s, int i);
void appendIntElement(struct IntSet* s, struct IntElement* e);
struct IntElement* popIntElement(struct IntSet* s);
struct IntSet* intersectIntSet(const struct IntSet* setA, const struct IntSet* setB);
