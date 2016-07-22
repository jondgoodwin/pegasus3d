/** Buffer: an array of raw atomic data (not Values), all of the same type
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"

/** Create a new Buffer value, with passed type and number of items. */
int buffer_new(Value th) {
	// Get itemtype and nitems parameters
	if (getTop(th)<2) {
		pushValue(th, aNull);
		return 1;
	}
	Value itemtype = pushMember(th, 1, "newtype"); // We want instance type, not constructor
	Value ic = pushMember(th, 3, "itemcount");
	Aint itemcount = isInt(ic) ? toAint(ic) : 3; // Number of values in an item
	Aint valspace = toAint(pushMember(th, 3, "valuespace")); // sizeof(instance)
	Aint nitems = 0;
	if (getTop(th)>2 && isInt(getLocal(th, 2)))
		nitems = toAint(getLocal(th, 2));

	// Create the buffer and initialize the header
	Value bufv = pushStringl(th, pushMember(th, 0, "newtype"), NULL, sizeof(BufferHeader) + nitems*valspace);
	BufferHeader* bufhdr = (BufferHeader*) toStr(bufv);
	bufhdr->itemtype = itemtype;
	bufhdr->nbrValues = 0;
	bufhdr->itemcount = itemcount;
	bufhdr->valspace = valspace;
	return 1;
}

/* Append a single value to the end of the buffer */
int buffer_append(Value th) {
	if (getTop(th)<2)
		return 1;
	BufferHeader* bufhdr = (BufferHeader*) toStr(getLocal(th, 0));
	if ((getSize(getLocal(th,0))-sizeof(BufferHeader))/bufhdr->valspace <= bufhdr->nbrValues)
		return 1;
	GLfloat *bufp = &((GLfloat*)(bufhdr+1))[bufhdr->nbrValues];
	*bufp = toAfloat(getLocal(th, 1));
	bufhdr->nbrValues++;
	return 1;
}

/** Initialize Xyz type and mixin */
void buffer_init(Value th) {
	Value Xyz = pushType(th, aNull, 2);
		pushMixin(th, aNull, aNull, 16);
			pushCMethod(th, buffer_append);
			popMember(th, 1, "<<");
		popMember(th, 0, "newtype");
		pushCMethod(th, buffer_new);
		popMember(th, 0, "new");
	popGloVar(th, "Buffer");
}