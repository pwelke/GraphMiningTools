/* from http://stackoverflow.com/questions/16826422/c-most-efficient-way-to-convert-string-to-int-faster-than-atoi */
static unsigned int digit_value (char c)
{
   return (unsigned int)(c - '0');
}

char is_digit (char c)
{
   return digit_value(c) <= 9;
}

/* from http://stackoverflow.com/questions/16826422/c-most-efficient-way-to-convert-string-to-int-faster-than-atoi */
static unsigned int extract_uint (char const **read_ptr)
{
   char const *p = *read_ptr;
   unsigned int d;
   unsigned int n;

   for ( ; isspace(*p); p++) {}
   
   n = digit_value(*p);
   while ((d = digit_value(*++p)) <= 9)
   {
      n = n * 10 + d;
   }

   *read_ptr = p;

   return n;
}

static inline int parseHeaderNew(int* id, int* activity, int* n, int* m) { 
	char const * current = *HEAD_PTR + 1;
	// fprintf(stderr, "starting at:%c:\n", *current);
	if ((*HEAD_PTR)[0] != '#') { 
		// fprintf(stderr, "did not find #\n");
		return -1; 
	}
	*id = extract_uint(&current);
	*activity = extract_uint(&current);
	*n = extract_uint(&current);
	*m = extract_uint(&current); 
	// fprintf(stderr, "\n");
	return (*id != -1) + (*activity != -1) + (*n != -1) + (*m != -1); // C11(ISO/IEC 9899:201x) §6.5.8 Relational operators
}


static inline int parseEdgeNewNew(char const ** currentPosition, int* v, int* w, char** label) {
	*v = extract_uint(currentPosition);
	*w = extract_uint(currentPosition);
	grabLabel(currentPosition, label);
	return (*v != -1) + (*w != -1) + (*label != NULL);
}