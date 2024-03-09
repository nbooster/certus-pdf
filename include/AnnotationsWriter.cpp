#include "AnnotationsWriter.h"

#include "../libs/PDF-Writer-master/PDFWriter/ObjectsContext.h"
#include "../libs/PDF-Writer-master/PDFWriter/ETokenSeparator.h"


AnnotationsWriter::AnnotationsWriter(void) {}

void AnnotationsWriter::AddCopiedAnnotation(ObjectIDType inNewAnnotation)
{
    mAnnotationsIDs.push_back(inNewAnnotation);
}

EStatusCode AnnotationsWriter::OnPageWrite
(
    PDFPage* inPage,
    DictionaryContext* inPageDictionaryContext,
    ObjectsContext* inPDFWriterObjectContext,
    DocumentContext* inPDFWriterDocumentContext
)
{
    if(mAnnotationsIDs.size() == 0)
        return eSuccess;

    if(inPageDictionaryContext->WriteKey("Annots") not_eq eSuccess)
        return eFailure;

    auto it = mAnnotationsIDs.begin();

    inPDFWriterObjectContext->StartArray();

    for(; it not_eq mAnnotationsIDs.end(); it++)
        inPDFWriterObjectContext->WriteIndirectObjectReference(*it);

    inPDFWriterObjectContext->EndArray(eTokenSeparatorEndLine);
    
    mAnnotationsIDs.clear();

    return eSuccess;
}