#include <algorithm>
#include <numeric>
#include <thread>
#include <mutex>

#include "../libs/PDF-Writer-master/PDFWriter/PDFPage.h"
#include "../libs/PDF-Writer-master/PDFWriter/PDFArray.h"
#include "../libs/PDF-Writer-master/PDFWriter/PDFWriter.h"
#include "../libs/PDF-Writer-master/PDFWriter/PDFUsedFont.h"
#include "../libs/PDF-Writer-master/PDFWriter/PDFObjectCast.h"
#include "../libs/PDF-Writer-master/PDFWriter/PDFFormXObject.h"
#include "../libs/PDF-Writer-master/PDFWriter/PageContentContext.h"
#include "../libs/PDF-Writer-master/PDFWriter/InputByteArrayStream.h"
#include "../libs/PDF-Writer-master/PDFWriter/PDFDocumentCopyingContext.h"
#include "../libs/PDF-Writer-master/PDFWriter/PDFIndirectObjectReference.h"

#include "./pdfBuffer.h"
#include "./helpers.h"
#include "./AnnotationsWriter.h"

#include "certusPDF.h"

static std::mutex finalPDFMutex;

static void drawLine
(
    PDFWriter& pdfWriter, 
    PDFPage* pdfPage, 
    const std::string& fromX = "0", 
    const std::string& fromY = "0", 
    const std::string& toX = "1", 
    const std::string& toY = "1", 
    const std::string& width = "1",
    const std::string& hexRGBColorValue = "000000",
    const std::string& opacity = std::to_string(NO_OPACITY_VALUE)
    )
{
    PageContentContext* pageContentContext = pdfWriter.StartPageContentContext(pdfPage); 

    pageContentContext->q();

    pageContentContext->SetupColor(
        { PageContentContext::eStroke, PageContentContext::eRGB, std::stoul(hexRGBColorValue, nullptr, 16), 1.0, false, std::stod(opacity) });

    pageContentContext->w(std::stod(width));

    pageContentContext->m(std::stod(fromX), std::stod(fromY));

    pageContentContext->l(std::stod(toX), std::stod(toY));

    pageContentContext->s();
    
    pageContentContext->Q();

    pdfWriter.EndPageContentContext(pageContentContext);
}

static void writeText
(
    PDFWriter& pdfWriter, 
    PDFPage* pdfPage, 
    PDFUsedFont* font,
    const std::string& line = "", 
    const std::string& startX = "0", 
    const std::string& startY = "0", 
    const std::string& fontSize = "10", 
    const std::string& textHexRGBColorValue = "000000", 
    const std::string& textOpacity = std::to_string(NO_OPACITY_VALUE),
    const std::string& highlightHexRGBColorValue = "FFFFFF", 
    const std::string& highlightOpacity = std::to_string(NO_OPACITY_VALUE),
    const bool highlight = false,
    const std::string& endX = "",
    const bool underlined = false,
    const std::string& hyperlink = ""
    )
{
    PageContentContext* pageContentContext = pdfWriter.StartPageContentContext(pdfPage);

    PDFUsedFont::TextMeasures textDimensions;

    const auto doubleStartX = std::stod(startX);
    const auto doubleStartY = std::stod(startY);
    const auto longFontSize = std::stol(fontSize);

    bool dimensionsCalculated = false;
    
    if (endX == "")
    {
        if (highlight)
        {   
            textDimensions = font->CalculateTextDimensions(line, longFontSize);
            dimensionsCalculated = true;

            pageContentContext->DrawRectangle(doubleStartX + textDimensions.xMin, doubleStartY + textDimensions.yMin, textDimensions.width, textDimensions.height, 
                { PageContentContext::eFill, PageContentContext::eRGB, std::stoul(highlightHexRGBColorValue, nullptr, 16), 1.0, false, std::stod(highlightOpacity) });
        }

        pageContentContext->WriteText(doubleStartX, doubleStartY, line, 
            { font, static_cast<double>(longFontSize), PageContentContext::eRGB, std::stoul(textHexRGBColorValue, nullptr, 16), std::stod(textOpacity) });

        if (underlined)
        {
            if ( not dimensionsCalculated)
            {
                textDimensions = font->CalculateTextDimensions(line, longFontSize);
                dimensionsCalculated = true;
            }

            const auto lineY = std::to_string(doubleStartY + sGetUnderlinePositionFactor(font) * longFontSize);

            pdfWriter.EndPageContentContext(pageContentContext);

            drawLine(pdfWriter, pdfPage, std::to_string(doubleStartX + textDimensions.xMin), lineY, 
                std::to_string(doubleStartX + textDimensions.xMin + textDimensions.width/*font->CalculateTextAdvance(line, longFontSize)*/), lineY, 
                std::to_string(sGetUnderlineThicknessFactor(font) * longFontSize), textHexRGBColorValue, textOpacity);

            pageContentContext = pdfWriter.StartPageContentContext(pdfPage);
        }
    } 
    else
    {
        auto lineCopy = line;

        lineCopy.erase(std::unique(lineCopy.begin(), lineCopy.end(), [](char lhs, char rhs) { return lhs == rhs and std::isspace(lhs); }), lineCopy.end());

        const auto doubleEndX = std::stod(endX);

        if (doubleEndX > doubleStartX)
        {
            if ( not dimensionsCalculated)
            {
                textDimensions = font->CalculateTextDimensions(lineCopy, longFontSize);
                dimensionsCalculated = true;
            }

            if (highlight) 
                pageContentContext->DrawRectangle(doubleStartX + textDimensions.xMin, doubleStartY + textDimensions.yMin, (doubleEndX - doubleStartX - textDimensions.xMin), textDimensions.height, 
                    { PageContentContext::eFill, PageContentContext::eRGB, std::stoul(highlightHexRGBColorValue, nullptr, 16), 1.0, false, std::stod(highlightOpacity) });

            if (underlined)
            {
                const auto lineY = std::to_string(doubleStartY + sGetUnderlinePositionFactor(font) * longFontSize);

                pdfWriter.EndPageContentContext(pageContentContext);

                drawLine(pdfWriter, pdfPage, std::to_string(doubleStartX + textDimensions.xMin), lineY, endX, lineY, 
                    std::to_string(sGetUnderlineThicknessFactor(font) * longFontSize), textHexRGBColorValue, textOpacity);

                pageContentContext = pdfWriter.StartPageContentContext(pdfPage);
            }

            const auto words = stringSplit(lineCopy);

            std::vector<double> wordsWidth;

            std::transform(words.begin(), words.end(), std::back_inserter(wordsWidth), 
                [&font, longFontSize](const std::string& word){ return (font->CalculateTextDimensions(word, longFontSize)).width; });

            const auto wordsWidthSum = std::accumulate(wordsWidth.begin(), wordsWidth.end(), decltype(wordsWidth)::value_type(0));

            const auto spaceBetweenWords = ((doubleEndX - doubleStartX - textDimensions.xMin) - static_cast<double>(wordsWidthSum)) / static_cast<double>(words.size() - 1);

            if (spaceBetweenWords >= 0)
            {
                auto xPosition = doubleStartX;

                for(std::size_t index = 0; index < words.size(); ++index)
                {
                    pageContentContext->WriteText(xPosition, doubleStartY, words[index], 
                        { font, static_cast<double>(longFontSize), PageContentContext::eRGB, std::stoul(textHexRGBColorValue, nullptr, 16), std::stod(textOpacity) });

                    xPosition += spaceBetweenWords + static_cast<double>(wordsWidth[index]);
                }
            }
        }
    }

    pdfWriter.EndPageContentContext(pageContentContext);

    if (not hyperlink.empty() and stringIsAscii7(hyperlink))
    {
        if (not dimensionsCalculated)
        {
            textDimensions = font->CalculateTextDimensions(line, longFontSize);
            dimensionsCalculated = true;
        }

        std::string URL = hyperlink;

        stringTrimEdges(URL);
        
        pdfWriter.AttachURLLinktoCurrentPage( (URL.find("http") == std::string::npos) ? "http://" + URL : URL , 
            {doubleStartX + textDimensions.xMin, doubleStartY + textDimensions.yMin, 
            doubleStartX + textDimensions.xMin + textDimensions.width, doubleStartY + textDimensions.yMin + textDimensions.height});
    }
}

static void drawRectangle
(
    PDFWriter& pdfWriter, 
    PDFPage* pdfPage,
    const std::string& inLeft,
    const std::string& inBottom,
    const std::string& inWidth,
    const std::string& inHeight,
    const std::string& hexRGBColorValue = "000000",
    const std::string& opacity = std::to_string(NO_OPACITY_VALUE),
    const std::string& fill = "N"
    )
{
    PageContentContext* pageContentContext = pdfWriter.StartPageContentContext(pdfPage);

    pageContentContext->DrawRectangle(std::stod(inLeft), std::stod(inBottom), std::stod(inWidth), std::stod(inHeight), 
        { (fill == "N" ? PageContentContext::eStroke : PageContentContext::eFill), PageContentContext::eRGB, std::stoul(hexRGBColorValue, nullptr, 16), 1.0, false, std::stod(opacity) });

    pdfWriter.EndPageContentContext(pageContentContext);
}

static void drawCircle
(
    PDFWriter& pdfWriter, 
    PDFPage* pdfPage,
    const std::string& inCenterX,
    const std::string& inCenterY,
    const std::string& inRadius,
    const std::string& hexRGBColorValue = "000000",
    const std::string& opacity = std::to_string(NO_OPACITY_VALUE),
    const std::string& fill = "N"
    )
{
    PageContentContext* pageContentContext = pdfWriter.StartPageContentContext(pdfPage);

    pageContentContext->DrawCircle(std::stod(inCenterX), std::stod(inCenterY), std::stod(inRadius), 
        { (fill == "N" ? PageContentContext::eStroke : PageContentContext::eFill), PageContentContext::eRGB, std::stoul(hexRGBColorValue, nullptr, 16), 1.0, false, std::stod(opacity) });

    pdfWriter.EndPageContentContext(pageContentContext);
}

static void drawPolygon
(
    PDFWriter& pdfWriter, 
    PDFPage* pdfPage,
    const std::string& points,
    const std::string& hexRGBColorValue = "000000",
    const std::string& opacity = std::to_string(NO_OPACITY_VALUE),
    const std::string& fill = "N"
    )
{
    auto strPoints = stringSplit(points, certusPDF::delimiter);

    if (strPoints.size() % 2)
        return;

    DoubleAndDoublePairList doublePoints;

    for (std::size_t index = 0; index < strPoints.size(); index += 2)
        doublePoints.emplace_back(std::stod(strPoints[index]), std::stod(strPoints[index + 1]));

    PageContentContext* pageContentContext = pdfWriter.StartPageContentContext(pdfPage);

    pageContentContext->DrawPath(doublePoints,
        { (fill == "N" ? PageContentContext::eStroke : PageContentContext::eFill), PageContentContext::eRGB, std::stoul(hexRGBColorValue, nullptr, 16), 1.0, false, std::stod(opacity) });

    pdfWriter.EndPageContentContext(pageContentContext);
}

static void drawImage
(
    PDFWriter& pdfWriter, 
    PDFPage* pdfPage, 
    PDFFormXObject* image, 
    const std::string& startX = "0", 
    const std::string& startY = "0",
    const std::string& scaleX = "1.0", 
    const std::string& scaleY = "1.0",
    const std::string& opacity = std::to_string(NO_OPACITY_VALUE)
    )
{
    PageContentContext* pageContentContext = pdfWriter.StartPageContentContext(pdfPage);

    pageContentContext->q();

    pageContentContext->SetOpacity(std::stod(opacity));

    pageContentContext->cm(std::stod(scaleX), 0, 0, std::stod(scaleY), std::stoi(startX), std::stoi(startY));

    pageContentContext->Do(pdfPage->GetResourcesDictionary().AddFormXObjectMapping(image->GetObjectID()));

    pageContentContext->SetOpacity(NO_OPACITY_VALUE);

    pageContentContext->Q();

    pdfWriter.EndPageContentContext(pageContentContext);
}

static void writePage
(
    PDFWriter& pdfWriter, 
    PDFUsedFont* font, 
    const std::vector<std::string>& inputPage,
    const int dimX = 595,
    const int dimY = 842
    )
{
    PDFPage* pdfPage = new PDFPage();

    pdfPage->SetMediaBox(PDFRectangle(0, 0, dimX, dimY));

    std::vector<std::string> fields, textParts;

    std::size_t delimiterPos;

    std::string hyperlink = "";

    for (auto line : inputPage)
    {
        if (line.size() <= 11)
            continue;

        hyperlink = stringExtractHyperlink(line);

        switch (line[0])
        {
            case 'L':

                fields = stringSplit(line, certusPDF::delimiter);
                
                if (fields.size() == 8)
                    drawLine(pdfWriter, pdfPage, fields[1], fields[2], fields[3], fields[4], fields[5], fields[6], fields[7]);

                break;

            case 'R':

                fields = stringSplit(line, certusPDF::delimiter);

                if (fields.size() == 8)
                    drawRectangle(pdfWriter, pdfPage, fields[1], fields[2], fields[3], fields[4], fields[5], fields[6], fields[7]);

                break;

            case 'C':

                fields = stringSplit(line, certusPDF::delimiter);

                if (fields.size() == 7)
                    drawCircle(pdfWriter, pdfPage, fields[1], fields[2], fields[3], fields[4], fields[5], fields[6]);

                break;

            case 'P':

                delimiterPos = stringFindNthOccuranceIndex(line, certusPDF::delimiter, certusPDF::pointsDelimiterStartPosition);
                
                if (delimiterPos not_eq std::string::npos)
                {
                    fields = stringSplit(line.substr(0, delimiterPos), certusPDF::delimiter);
                    
                    if (fields.size() == 4)
                        drawPolygon(pdfWriter, pdfPage, line.substr(delimiterPos + 1), fields[1], fields[2], fields[3]);
                }
               
                break;

            case 'I':

                fields = stringSplit(line, certusPDF::delimiter);
                
                if (fields.size() == 8)
                {
                    PDFFormXObject* image = base64StringToImage(pdfWriter, fields[6], fields[7]);

                    drawImage(pdfWriter, pdfPage, image, fields[1], fields[2], fields[3], fields[4], fields[5]);
                    
                    delete image;
                }
                    
                break;

            case 'Q':

                fields = stringSplit(line, certusPDF::delimiter);
                
                if (fields.size() == 6)
                {
                    auto& text = fields[5];

                    stringTrimEdges(text);
                    
                    PDFFormXObject* image = stringToQRCodePNGImage(pdfWriter, text, std::stoi(fields[3]));
                    
                    drawImage(pdfWriter, pdfPage, image, fields[1], fields[2], "1.0", "1.0", fields[4]);

                    delete image;
                }

                break;
            
            case 'T':
                
                delimiterPos = stringFindNthOccuranceIndex(line, certusPDF::delimiter, certusPDF::textDelimiterStartPosition);
                
                if (delimiterPos not_eq std::string::npos)
                {
                    fields = stringSplit(line.substr(0, delimiterPos), certusPDF::delimiter);
                    
                    if (fields.size() == 6)
                        writeText(pdfWriter, pdfPage, font, line.substr(delimiterPos + 1), 
                            fields[1], fields[2], fields[3], fields[4], fields[5], "FFFFFF", std::to_string(NO_OPACITY_VALUE), false, "", line[1] == 'U', hyperlink);
                }
               
                break;

            case 'H':

                delimiterPos = stringFindNthOccuranceIndex(line, certusPDF::delimiter, certusPDF::textDelimiterStartPosition + 2);
                
                if (delimiterPos not_eq std::string::npos)
                {
                    fields = stringSplit(line.substr(0, delimiterPos), certusPDF::delimiter);
                    
                    if (fields.size() == 8)
                        writeText(pdfWriter, pdfPage, font, line.substr(delimiterPos + 1), 
                            fields[1], fields[2], fields[3], fields[4], fields[5], fields[6], fields[7], true, "", line[1] == 'U', hyperlink);
                }
               
                break;

            case 'A':

                delimiterPos = stringFindNthOccuranceIndex(line, certusPDF::delimiter, certusPDF::textDelimiterStartPosition + 1);
                
                if (delimiterPos not_eq std::string::npos)
                {
                    if (fields.size() == 7)
                        writeText(pdfWriter, pdfPage, font, line.substr(delimiterPos + 1), 
                            fields[1], fields[2], fields[4], fields[5], fields[6], "", "", false, fields[3], line[1] == 'U', hyperlink);
                }
               
                break;

            case 'W':

                delimiterPos = stringFindNthOccuranceIndex(line, certusPDF::delimiter, certusPDF::textDelimiterStartPosition + 3);
                
                if (delimiterPos not_eq std::string::npos)
                {
                    fields = stringSplit(line.substr(0, delimiterPos), certusPDF::delimiter);
                    
                    if (fields.size() == 9)
                        writeText(pdfWriter, pdfPage, font, line.substr(delimiterPos + 1), 
                            fields[1], fields[2], fields[4], fields[5], fields[6], fields[7], fields[8], true, fields[3], line[1] == 'U', hyperlink);
                }
               
                break;

            case 'S':

                fields = stringSplit(line, certusPDF::delimiter);
                
                if (fields.size() == 6 and stringIsAscii7(fields[5]))
                {
                    const auto URL = stringTrimEdges(fields[5]);

                    pdfWriter.AttachURLLinktoCurrentPage( (URL.find("http") == std::string::npos) ? "http://" + URL : URL , 
                        { std::stod(fields[1]), std::stod(fields[2]), std::stod(fields[3]), std::stod(fields[4]) });
                }

            default:

                break;
        }
    }
    
    pdfWriter.WritePageAndRelease(pdfPage);
}

static EStatusCode appendPageWithAnnotationsToPDFStream
(
    PDFWriter& inTargetWriter, 
    IByteReaderWithPosition* onePagePdfStreamBuffer
    )
{
    EStatusCode status = eSuccess;

    AnnotationsWriter annotationsWriter;
    
    inTargetWriter.GetDocumentContext().AddDocumentContextExtender(&annotationsWriter);

    auto copyingContext = inTargetWriter.CreatePDFCopyingContext(onePagePdfStreamBuffer);

    if ( not copyingContext)
        return eFailure;

    RefCountPtr<PDFDictionary> pageDictionary(copyingContext->GetSourceDocumentParser()->ParsePage(0));

    PDFObjectCastPtr<PDFArray> annotations(copyingContext->GetSourceDocumentParser()->QueryDictionaryObject(pageDictionary.GetPtr(), "Annots"));

    if(annotations.GetPtr())
    {
        SingleValueContainerIterator<PDFObjectVector> annotationDictionaryObjects = annotations->GetIterator();

        PDFObjectCastPtr<PDFIndirectObjectReference> annotationReference;

        while(annotationDictionaryObjects.MoveNext() and eSuccess == status)
        {
            annotationReference = annotationDictionaryObjects.GetItem();

            EStatusCodeAndObjectIDType result = copyingContext->CopyObject(annotationReference->mObjectID);

            status = result.first;

            if(eSuccess == status)
                annotationsWriter.AddCopiedAnnotation(result.second);
        }

        if(status not_eq eSuccess)
        {
            delete copyingContext;

            return status;
        }
    }

    copyingContext->AppendPDFPageFromPDF(0).first;

    delete copyingContext;

    inTargetWriter.GetDocumentContext().RemoveDocumentContextExtender(&annotationsWriter);

    return eSuccess;
}

static void createAppendOnePagePDF
(
    PDFWriter& pdfWriterFinal,
    const std::vector<std::string>& inputPage, 
    const std::string& fontFilePath, 
    const int dimX,
    const int dimY
    )
{
    PDFWriter pdfWriter;

    pdfBuffer pdfStreamBuffer;

    pdfWriter.StartPDFForStream(&pdfStreamBuffer, ePDFVersion13);

    PDFUsedFont* font = pdfWriter.GetFontForFile(fontFilePath);

    writePage(pdfWriter, font, inputPage, dimX, dimY);

    pdfWriter.EndPDFForStream();

    InputByteArrayStream IBAS(pdfStreamBuffer.data(), static_cast<long long>(pdfStreamBuffer.size()));
    
    finalPDFMutex.lock();

    appendPageWithAnnotationsToPDFStream(pdfWriterFinal, &IBAS);

    finalPDFMutex.unlock();
}

void certusPDF::createPDF
(
    const std::vector<std::vector<std::string>>& inputPages,
    const std::string& fontFilePath,
    const std::string& pdfFilePath,
    const int dimX,
    const int dimY
    )
{
    PDFWriter pdfWriterFinal;

    pdfWriterFinal.StartPDF(pdfFilePath, ePDFVersion13);

    std::vector<std::thread> outPdfBuffers;
    outPdfBuffers.reserve(inputPages.size());

    for (auto& inputPage : inputPages)
        outPdfBuffers.push_back( std::thread{ createAppendOnePagePDF, std::ref(pdfWriterFinal), std::ref(inputPage), std::ref(fontFilePath), dimX, dimY } ); 

    for (auto& outPdfBuffer : outPdfBuffers)
        outPdfBuffer.join();

    pdfWriterFinal.EndPDF();
}