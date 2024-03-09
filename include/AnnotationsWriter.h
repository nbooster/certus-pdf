#include <list>

#include "../libs/PDF-Writer-master/PDFWriter/PDFPage.h"
#include "../libs/PDF-Writer-master/PDFWriter/DictionaryContext.h"
#include "../libs/PDF-Writer-master/PDFWriter/DocumentContextExtenderAdapter.h"

class ObjectsContext;

class AnnotationsWriter : public DocumentContextExtenderAdapter
{

public:

    AnnotationsWriter(void);

    void AddCopiedAnnotation(ObjectIDType inNewAnnotation);

    EStatusCode OnPageWrite
    (
        PDFPage* inPage,
        DictionaryContext* inPageDictionaryContext,
        ObjectsContext* inPDFWriterObjectContext,
        DocumentContext* inPDFWriterDocumentContext
    );

private:

    std::list<ObjectIDType> mAnnotationsIDs;
};