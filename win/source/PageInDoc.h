#ifndef PAGE_IN_DOC_H_
#define PAGE_IN_DOC_H_


class PageInDoc {
private:
	int32 spreadIndex;
	int32 inSpreadIndex;

public:
	PageInDoc(int32 spreadIndex, int32 inSpreadIndex);

	/**
	 * Constructs a new object from absolute page index(starts from 1) in document,
	 * and intervals vector nessesary for parsing.
	 * Returns objects with (-1;-1) field values if invalid data in params.
	 */
	PageInDoc(int32 absPageIndex, std::vector<int32> spreadPagesIntervals);

	int32 getSpreadIndex();
	int32 getInSpreadIndex();
};

#endif
