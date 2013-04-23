/* This file is part of 34S.
 *
 * 34S is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 34S is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 34S.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QTSKIN_H_
#define QTSKIN_H_

#include <exception>
#include <QtGui>
#include <QStack>
#include <QMap>
#include <QLinkedList>
#include <QtXml>
#include <QColor>
#include "QtKey.h"
#include "QtScreenPainter.h"

class QtSkinException: public std::exception
{
public:
	QtSkinException(const QString& anErrorMessage);
	~QtSkinException() throw();
	virtual const char* what() const throw();

public:
	QString errorMessage;
};

class QtSkin;
class TagHandler;

#define KEY_COUNT 37
#define MAX_KEY_CODE 41
#define DOT_PAINTERS_COUNT 400
#define PASTE_PAINTERS_COUNT DOT_PAINTERS_COUNT

typedef QHash<QString, TagHandler> TagHandlers;
typedef bool (QtSkin::*StartElementMethod)(const QString&, const QXmlAttributes&);
typedef bool (QtSkin::*EndElementMethod)(const QString&);
typedef bool (QtSkin::*CharactersMethod)(const QString&);
typedef QVector<QtKey*> QtKeyList;
typedef QtKeyList::const_iterator QtKeyConstIterator;
typedef QVector<DotPainter*> DotPainterList;
typedef DotPainterList::const_iterator DotPainterListConstIterator;

class QtSkin: public QXmlDefaultHandler
{
public: // Constructors and parsing methods
	QtSkin(QFile& aFile) throw (QtSkinException);
    bool startElement(const QString& aNamespaceURI, const QString& aLocalName,
                      const QString& aName, const QXmlAttributes& theAttributes);
    bool endElement(const QString& aNamespaceURI, const QString& aLocalName,
                    const QString& aName);
    bool characters(const QString& aString);

    bool fatalError(const QXmlParseException& anException);
    void setDocumentLocator(QXmlLocator* aLocator);
    QString errorString() const;
    bool ignoreCharacters(const QString& aString);

public: // getters for Skin fields
    QString getPictureName() const;
    const QSize& getPictureSize() const;
    const QRect& getScreenRectangle() const;
    const QRect& getPasteRectangle() const;
    const QColor& getSCreenForeground() const;
    const QColor& getSCreenBackground() const;
    int getHShiftHeight() const;
    QPoint getNumberPosition() const;
    int getNumberFontSize() const;
    int getNumberFontStretch() const;
    int getNumberExtraWidth() const;
    int getSeparatorShift() const;
    QPoint getExponentPosition() const;
    int getExponentFontSize() const;
    int getExponentFontStretch() const;
    QPoint getTextPosition() const;
    QSize getTextSize() const;
    int getFontSize() const;
    int getFontLowerSize() const;
    int getFontStretch() const;
    int getSmallFontSize() const;
    int getSmallFontLowerSize() const;
    int getSmallFontStretch() const;
    const QtKeyList& getKeys() const;
    const DotPainterList getDotPainters() const;
    const DotPainterList getPastePainters() const;

public: // Parsing methods
    bool startSkin(const QString& aName, const QXmlAttributes& theAttributes);
    bool startPicture(const QString& aName, const QXmlAttributes& theAttributes);
    bool startSize(const QString& aName, const QXmlAttributes& theAttributes);
    bool startScreen(const QString& aName, const QXmlAttributes& theAttributes);
    bool startForeground(const QString& aName, const QXmlAttributes& theAttributes);
    bool startBackground(const QString& aName, const QXmlAttributes& theAttributes);
    bool startHShift(const QString& aName, const QXmlAttributes& theAttributes);
    bool startNumber(const QString& aName, const QXmlAttributes& theAttributes);
    bool startExponent(const QString& aName, const QXmlAttributes& theAttributes);
    bool startText(const QString& aName, const QXmlAttributes& theAttributes);
    bool startSmallText(const QString& aName, const QXmlAttributes& theAttributes);
    bool startKeys(const QString& aName, const QXmlAttributes& theAttributes);
    bool startKey(const QString& aName, const QXmlAttributes& theAttributes);
    bool startShortcut(const QString& aName, const QXmlAttributes& theAttributes);
    bool startPainters(const QString& aName, const QXmlAttributes& theAttributes);
    bool startPasters(const QString& aName, const QXmlAttributes& theAttributes);
    bool startPainter(const QString& aName, const QXmlAttributes& theAttributes);
    bool charactersPolygon(const QString& aName);
    bool startCopy(const QString& aName, const QXmlAttributes& theAttributes);

private: // Internal use methods
    void setSimpleErrorMessage(const QString& anErrorMessage);
    void setErrorMessage(const QString& anErrorMessage);
    void setErrorMessage(const QString& anErrorMessage, int aLine, int aColumn);
    void pushHandlers(const TagHandlers& theTagHandlers);
    void popStacks();
    bool forwardToCharactersMethods();
    bool checkSkin();
    bool convertStringToInteger(const QString& aString, int& anInteger);
    bool convertAttributesToColor(const QString& aName, const QXmlAttributes& theAttributes, QColor& aColor);
    bool convertAttributesToRectangle(const QString& aName, const QXmlAttributes& theAttibutes, QRect& aRectangle);

private: // Skin fields
	QString pictureName;
	QSize pictureSize;
	QRect screenRectangle;
	QRect pasteRectangle;
    QColor screenForeground;
    QColor screenBackground;
    int hShiftHeight;
    QPoint numberPosition;
    int numberFontSize;
    int numberFontStretch;
    int numberExtraWidth;
    int separatorShift;
    QPoint exponentPosition;
    int exponentFontSize;
    int exponentFontStretch;
    QPoint textPosition;
    QSize textSize;
    int fontSize;
    int fontLowerSize;
    int fontStretch;
    int smallFontSize;
    int smallFontLowerSize;
    int smallFontStretch;
    QtKeyList keys;
    DotPainterList dotPainters;
    DotPainterList pastePainters;
    DotPainterList* currentPainters;

private: // Internal use fields
	QString errorMessage;
	QXmlLocator* locator;
	QStack<TagHandlers> handlersStack;
	QStack<CharactersMethod> characterMethodsStack;
	QString currentCharacters;
	int currentCharactersLine, currentCharactersColumn;
    int insertedKeys, insertedDotPainters, insertedPastePainters, *insertedPainters;
    QtKey* currentKey;
    DotPainter* currentDotPainter;
};


// A few other classes
class TagHandler
{
public:
	TagHandler(StartElementMethod aStartElementMethod,
			EndElementMethod anEndElementMethod,
			CharactersMethod aCharactersMethod);
	TagHandler();

public:
	StartElementMethod startElementMethod;
	EndElementMethod endElementMethod;
	CharactersMethod charactersMethod;
};

class DocumentHandlers: public TagHandlers
{
public:
	DocumentHandlers();
};

class SkinHandlers: public TagHandlers
{
public:
	SkinHandlers();
};

class KeysHandlers: public TagHandlers
{
public:
	KeysHandlers();
};

class KeyHandlers: public TagHandlers
{
public:
	KeyHandlers();
};

class PaintersHandlers: public TagHandlers
{
public:
	PaintersHandlers();
};

class PainterHandlers: public TagHandlers
{
public:
	PainterHandlers();
};


#endif /* QTSKIN_H_ */
