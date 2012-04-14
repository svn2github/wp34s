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

#include <stack>
#include "QtSkin.h"

static const DocumentHandlers documentHandlers;
static const SkinHandlers skinHandlers;
static const TagHandlers emptyHandlers;
static const KeysHandlers keysHandlers;
static const KeyHandlers keyHandlers;
static const PaintersHandlers paintersHandlers;
static const PainterHandlers painterHandlers;


QtSkin::QtSkin(QFile& aFile) throw (QtSkinException)
		: pictureSize(-1, -1), screenRectangle(-1, -1, -1, -1), screenForeground(), screenBackground(), hShiftHeight(0),
		  keys(MAX_KEY_CODE, NULL), dotPainters(DOT_PAINTERS_COUNT, NULL), pastePainters(PASTE_PAINTERS_COUNT, NULL),
		  insertedKeys(0), insertedDotPainters(0), insertedPastePainters(0)
{
	pushHandlers(documentHandlers);
	characterMethodsStack.push(NULL);
	QXmlSimpleReader reader;
	reader.setContentHandler(this);
	reader.setErrorHandler(this);

    QXmlInputSource xmlInputSource(&aFile);
    if(!reader.parse(xmlInputSource) || !checkSkin())
    {
    	throw *(new QtSkinException(errorString()));
    }
}

bool QtSkin::checkSkin()
{
	// TODO
	if(pictureName.isEmpty())
	{
		setSimpleErrorMessage("No pictureName");
		return false;
	}
	if(pictureSize.width()<0 || pictureSize.height()<0)
	{
		setSimpleErrorMessage("picture size not defined");
		return false;
	}
	if(screenRectangle.top()<0 || screenRectangle.left()<0 || screenRectangle.width()<0 || screenRectangle.height()<0)
	{
		setSimpleErrorMessage("screen rectangle not defined");
		return false;
	}
	if(!screenForeground.isValid())
	{
		setSimpleErrorMessage("screen foreground color not defined");
		return false;
	}
	if(!screenBackground.isValid())
	{
		setSimpleErrorMessage("screen background color not defined");
		return false;
	}
	if(insertedKeys!=KEY_COUNT)
	{
		setSimpleErrorMessage("Invalid number of keys: "+QString().setNum(insertedKeys)+", should be "+QString().setNum(KEY_COUNT));
		return false;
	}
	if(insertedDotPainters!=DOT_PAINTERS_COUNT)
	{
		setSimpleErrorMessage("Invalid number of painters: "+QString().setNum(insertedDotPainters)+", should be "+QString().setNum(DOT_PAINTERS_COUNT));
		return false;
	}
	if(insertedPastePainters!=0 && insertedPastePainters!=PASTE_PAINTERS_COUNT)
	{
		setSimpleErrorMessage("Invalid number of Paste painters: "+QString().setNum(insertedPastePainters)+", should be 0 or "+QString().setNum(PASTE_PAINTERS_COUNT));
		return false;
	}
	return true;
}

QString QtSkin::getPictureName() const
{
	return pictureName;
}

const QSize& QtSkin::getPictureSize() const
{
	return pictureSize;
}

const QRect& QtSkin::getScreenRectangle() const
{
	return screenRectangle;
}

const QRect& QtSkin::getPasteRectangle() const
{
	return pasteRectangle;
}

const QColor& QtSkin::getSCreenForeground() const
{
	return screenForeground;
}

const QColor& QtSkin::getSCreenBackground() const
{
	return screenBackground;
}

int QtSkin::getHShiftHeight() const
{
	return hShiftHeight;
}

const QtKeyList& QtSkin::getKeys() const
{
	return keys;
}

const DotPainterList QtSkin::getDotPainters() const
{
	return dotPainters;
}

const DotPainterList QtSkin::getPastePainters() const
{
	return pastePainters;
}


bool QtSkin::convertStringToInteger(const QString& aString, int& anInteger)
{
	bool valid;
	int value=aString.toInt(&valid);
	if(valid)
	{
		anInteger=value;
	}
	return valid;
}

bool QtSkin::convertAttributesToColor(const QString& aName, const QXmlAttributes& theAttributes, QColor& aColor)
{
	int redIndex=theAttributes.index("red");
	int greenIndex=theAttributes.index("green");
	int blueIndex=theAttributes.index("blue");

	if(redIndex<0 || greenIndex<0 || blueIndex<0)
	{
		setErrorMessage("Invalid attributes for color "+aName);
		return false;
	}
	else
	{
		int red, green, blue;
		if(!convertStringToInteger(theAttributes.value(redIndex), red))
		{
			setErrorMessage("Invalid red: "+theAttributes.value(redIndex)+" for "+aName);
			return false;
		}
		if(!convertStringToInteger(theAttributes.value(greenIndex), green))
		{
			setErrorMessage("Invalid green: "+theAttributes.value(greenIndex)+" for "+aName);
			return false;
		}
		if(!convertStringToInteger(theAttributes.value(blueIndex), blue))
		{
			setErrorMessage("Invalid blue: "+theAttributes.value(blueIndex)+" for "+aName);
			return false;
		}
		aColor.setRgb(red, green, blue);
	}

	return true;
}

bool QtSkin::convertAttributesToRectangle(const QString& aName, const QXmlAttributes& theAttributes, QRect& aRectangle)
{
	int xIndex = theAttributes.index("x");
	int yIndex = theAttributes.index("y");
	int widthIndex = theAttributes.index("width");
	int heightIndex = theAttributes.index("height");

	if (xIndex<0 || yIndex<0 || widthIndex < 0 || heightIndex < 0)
	{
		setErrorMessage("Invalid rectangle attributes for "+aName);
		return false;
	}
	else
	{
		int x;
		if (!convertStringToInteger(theAttributes.value(xIndex), x))
		{
			setErrorMessage("Invalid x " + theAttributes.value(xIndex)+ " for "+aName);
			return false;
		}
		int y;
		if (!convertStringToInteger(theAttributes.value(yIndex), y))
		{
			setErrorMessage("Invalid y " + theAttributes.value(yIndex)+ " for "+aName);
			return false;
		}
		int width;
		if (!convertStringToInteger(theAttributes.value(widthIndex), width))
		{
			setErrorMessage("Invalid witdh " + theAttributes.value(widthIndex)+ " for "+aName);
			return false;
		}
		int height;
		if (!convertStringToInteger(theAttributes.value(heightIndex), height))
		{
			setErrorMessage("Invalid height " + theAttributes.value(heightIndex)+ " for "+aName);
			return false;
		}
		aRectangle.setRect(x, y, width, height);
		return true;
	}
}

void QtSkin::pushHandlers(const TagHandlers& theTagHandlers)
{
	handlersStack.push(theTagHandlers);
}

void QtSkin::popStacks()
{
	handlersStack.pop();
	characterMethodsStack.pop();
}

bool QtSkin::forwardToCharactersMethods()
{
	currentCharacters=currentCharacters.trimmed();
	CharactersMethod charactersMethod=characterMethodsStack.top();
	if(charactersMethod!=NULL)
	{
		bool value=(this->*(charactersMethod))(currentCharacters);
		currentCharacters="";
		return value;
	}
	else
	{
		if(currentCharacters.isEmpty())
		{
			return true;
		}
		else
		{
			setErrorMessage("Unexpected characters", currentCharactersLine, currentCharactersColumn);
			return false;
		}
	}
}

bool QtSkin::ignoreCharacters(const QString& aString)
{
	Q_UNUSED(aString)

	return true;
}

bool QtSkin::startElement(const QString& aNamespaceURI, const QString& aLocalName,
                  const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aNamespaceURI)
	Q_UNUSED(aLocalName)

	if(handlersStack.isEmpty() || characterMethodsStack.isEmpty())
	{
		setErrorMessage("No handlers, stack is empty");
		return false;
	}

	if(!forwardToCharactersMethods())
	{
		return false;
	}

	TagHandlers& tagHandlers=handlersStack.top();
	if(tagHandlers.contains(aName))
	{
		TagHandler tagHandler=tagHandlers[aName];
		characterMethodsStack.push(tagHandler.charactersMethod);
		if(tagHandler.startElementMethod!=NULL)
		{
			int stackCount=handlersStack.count();
			bool value=(this->*(tagHandler.startElementMethod))(aName, theAttributes);
			int newStackCount=handlersStack.count();
			if(newStackCount==stackCount)
			{
				pushHandlers(emptyHandlers);
				return value;
			}
			else if(newStackCount!=stackCount+1)
			{
				setErrorMessage("Invalid handler stack modification for tag "+aName);
				return false;
			}
			return true;
		}
		else
		{
			if(theAttributes.count()!=0)
			{
				setErrorMessage("Invalid attributes for tag "+aName);
				return false;
			}

			pushHandlers(emptyHandlers);
			return true;
		}
	}
	else
	{
		setErrorMessage("Unknown tag \""+aName+'"');
		return false;
	}
}

bool QtSkin::endElement(const QString& aNamespaceURI, const QString& aLocalName,
                const QString& aName)
{
	Q_UNUSED(aNamespaceURI)
	Q_UNUSED(aLocalName)

	if(!forwardToCharactersMethods())
	{
		return false;
	}

	popStacks();
	if(handlersStack.isEmpty() || characterMethodsStack.isEmpty())
	{
		setErrorMessage("No handlers, stack is empty");
		return false;
	}

	TagHandlers& tagHandlers=handlersStack.top();

	if(tagHandlers.contains(aName))
	{
		EndElementMethod endElementMethod=(tagHandlers[aName]).endElementMethod;
		if(endElementMethod!=NULL)
		{
			int stackCount=handlersStack.count();
			bool value=(this->*endElementMethod)(aName);
			int newStackCount=handlersStack.count();
			if(newStackCount==stackCount)
			{
				return value;
			}
			else
			{
				setErrorMessage("Invalid handler stack modification in endElement for tag "+aName);
				return false;
			}
		}
		else
		{
			return true;
		}
	}
	else
	{
		setErrorMessage("Unknow ending tag \""+aName+'"');
		return false;
	}
}

bool QtSkin::characters(const QString& aString)
{
	if(currentCharacters.isEmpty())
	{
		currentCharactersLine=locator->lineNumber();
		currentCharactersColumn=locator->columnNumber();
	}
	currentCharacters+=aString;
	return true;
}

void QtSkin::setSimpleErrorMessage(const QString& anErrorMessage)
{
	errorMessage=anErrorMessage;
}

void QtSkin::setErrorMessage(const QString& anErrorMessage)
{
	setErrorMessage(anErrorMessage, locator->lineNumber(), locator->columnNumber());
}

void QtSkin::setErrorMessage(const QString& anErrorMessage, int aLine, int aColumn)
{
	errorMessage=anErrorMessage+", line "+QString::number(aLine)+", column "+QString::number(aColumn);
}

bool QtSkin::fatalError(const QXmlParseException &anException)
{
	setErrorMessage(anException.message(), anException.lineNumber(), anException.columnNumber());
	return false;
}

void QtSkin::setDocumentLocator(QXmlLocator* aLocator)
{
	locator=aLocator;
}

QString QtSkin::errorString() const
{
	return errorMessage;
}

bool QtSkin::startSkin(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)
	Q_UNUSED(theAttributes)

	pushHandlers(skinHandlers);
	return true;
}

bool QtSkin::startPicture(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)

	int index=theAttributes.index("name");
	if(index<0 || theAttributes.count()!=1)
	{
		setErrorMessage("Invalid attributes for picture");
		return false;
	}
	else
	{
		pictureName=theAttributes.value(index);
	}
	return true;
}

bool QtSkin::startSize(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)

	int widthIndex=theAttributes.index("width");
	int heightIndex=theAttributes.index("height");

	if(widthIndex<0 || heightIndex<0 || theAttributes.count()!=2)
	{
		setErrorMessage("Invalid attributes for size");
		return false;
	}
	else
	{
		if(!convertStringToInteger(theAttributes.value(widthIndex), pictureSize.rwidth()))
		{
			setErrorMessage("Invalid witdh "+theAttributes.value(widthIndex));
			return false;
		}
		if(!convertStringToInteger(theAttributes.value(heightIndex), pictureSize.rheight()))
		{
			setErrorMessage("Invalid height "+theAttributes.value(heightIndex));
			return false;
		}
	}

	return true;
}

bool QtSkin::startScreen(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)

	if (theAttributes.count() != 4)
	{
		setErrorMessage("Invalid attributes count for screen");
		return false;
	}
	return convertAttributesToRectangle("screen", theAttributes, screenRectangle);
}

bool QtSkin::startForeground(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)

	if(theAttributes.count()!=3)
	{
		setErrorMessage("Invalid attributes count for foreground");
	}
	return convertAttributesToColor("foreground", theAttributes, screenForeground);
}

bool QtSkin::startBackground(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)

	if(theAttributes.count()!=3)
	{
		setErrorMessage("Invalid attributes count for background");
	}
	return convertAttributesToColor("background", theAttributes, screenBackground);
}


bool QtSkin::startHShift(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)

	int heightIndex=theAttributes.index("height");

	if(heightIndex<0 || theAttributes.count()!=1)
	{
		setErrorMessage("Invalid attributes for hshift");
		return false;
	}
	else
	{
		if(!convertStringToInteger(theAttributes.value(heightIndex), hShiftHeight))
		{
			setErrorMessage("Invalid height "+theAttributes.value(heightIndex));
			return false;
		}
	}

	return true;
}

bool QtSkin::startKeys(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)
	Q_UNUSED(theAttributes)

	pushHandlers(keysHandlers);
	return true;
}

bool QtSkin::startKey(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)

	if(theAttributes.count()!=5)
	{
		setErrorMessage("Invalid attributes count for key");
	}
	int codeIndex=theAttributes.index("code");

	if(codeIndex<0)
	{
		setErrorMessage("Missing code attribute for key");
		return false;
	}
	else
	{
		int code;
		if(!convertStringToInteger(theAttributes.value(codeIndex), code))
		{
			setErrorMessage("Invalid code for key");
			return false;
		}

		QRect rectangle;
		if(!convertAttributesToRectangle("Key", theAttributes, rectangle))
		{
			return false;
		}

		if(code>=MAX_KEY_CODE)
		{
			setErrorMessage("Invalid key code "+QString().setNum(code)+", should be lesser than "+QString().setNum(MAX_KEY_CODE));
			return false;
		}
		if(keys[code]!=NULL)
		{
			setErrorMessage("Duplicated key code "+QString().setNum(code));
			return false;
		}
		currentKey=new QtKey(code, rectangle);
		keys[code]=currentKey;
		insertedKeys++;

		pushHandlers(keyHandlers);
		return true;
	}
}

bool QtSkin::startShortcut(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)

	if(theAttributes.count()!=5)
	{
		setErrorMessage("Invalid attributes count for shortcut");
	}
	int sequenceIndex=theAttributes.index("sequence");

	if(sequenceIndex<0)
	{
		setErrorMessage("Missing sequence attribute for shortcut");
		return false;
	}
	else
	{
		QKeySequence keySequence(theAttributes.value(sequenceIndex));
		currentKey->addKeySequence(keySequence);
		return true;
	}
}

bool QtSkin::startPainters(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)
	Q_UNUSED(theAttributes)

	pushHandlers(paintersHandlers);
	currentPainters=&dotPainters;
	insertedPainters=&insertedDotPainters;
	return true;
}

bool QtSkin::startPasters(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)
	Q_UNUSED(theAttributes)

	int widthIndex = theAttributes.index("width");
	int heightIndex = theAttributes.index("height");

	if (widthIndex < 0 || heightIndex < 0 || theAttributes.count()!=2)
	{
		setErrorMessage("Invalid attributes for Pasters");
		return false;
	}
	int width;
	if (!convertStringToInteger(theAttributes.value(widthIndex), width))
	{
		setErrorMessage("Invalid witdh " + theAttributes.value(widthIndex)+ " for Pasters");
		return false;
	}
	int height;
	if (!convertStringToInteger(theAttributes.value(heightIndex), height))
	{
		setErrorMessage("Invalid height " + theAttributes.value(heightIndex)+ " for Pasters");
		return false;
	}

	pasteRectangle.setRect(0, 0, width, height);
	pushHandlers(paintersHandlers);
	currentPainters=&pastePainters;
	insertedPainters=&insertedPastePainters;
	return true;
}

bool QtSkin::startPainter(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)


	if(theAttributes.count()!=1)
	{
		setErrorMessage("Invalid attributes count for painter");
	}
	int indexIndex=theAttributes.index("index");

	if(indexIndex<0)
	{
		setErrorMessage("Missing code attribute for painter");
		return false;
	}
	else
	{
		int index;
		if(!convertStringToInteger(theAttributes.value(indexIndex), index))
		{
			setErrorMessage("Invalid code for painter");
			return false;
		}

		if(index>=DOT_PAINTERS_COUNT)
		{
			setErrorMessage("Invalid painter index "+QString().setNum(index)+", should be lesser than "+QString().setNum(DOT_PAINTERS_COUNT));
			return false;
		}
		if((*currentPainters)[index]!=NULL)
		{
			setErrorMessage("Duplicated painter index "+QString().setNum(index));
			return false;
		}
		currentDotPainter=new DotPainter();
		(*currentPainters)[index]=currentDotPainter;
		(*insertedPainters)++;

		pushHandlers(painterHandlers);
		return true;
	}
}

bool QtSkin::charactersPolygon(const QString& aString)
{
	QStringList list=aString.split(',', QString::SkipEmptyParts);
	if(list.count()%2!=0)
	{
		setErrorMessage("Polygon count is not even");
		return false;
	}
    for (QStringList::const_iterator iterator = list.constBegin(); iterator != list.constEnd(); ++iterator)
    {
    	// dummy to avoid a warning
    	QString dummy=iterator->trimmed();
    }

    QPolygon polygon;
    for(int i=0; i<list.count(); i+=2)
    {
    	int x;
    	if(!convertStringToInteger(list[i], x))
    	{
    		setErrorMessage("Invalid number "+list[i]+" in polygon");
    		return false;
    	}
    	int y;
    	if(!convertStringToInteger(list[i+1], y))
    	{
    		setErrorMessage("Invalid number "+list[i+1]+" in polygon");
    		return false;
    	}
    	polygon.append(QPoint(x, y));
    }

	PolygonPainter* polygonPainter=new PolygonPainter(polygon);
	currentDotPainter->addLCDPainter(polygonPainter);

	return true;
}

bool QtSkin::startCopy(const QString& aName, const QXmlAttributes& theAttributes)
{
	Q_UNUSED(aName)

	int destinationXIndex = theAttributes.index("destinationX");
	int destinationYIndex = theAttributes.index("destinationY");
	int widthIndex = theAttributes.index("width");
	int heightIndex = theAttributes.index("height");
	int sourceXIndex = theAttributes.index("sourceX");
	int sourceYIndex = theAttributes.index("sourceY");

	if (destinationXIndex<0 || destinationYIndex<0 || widthIndex < 0 || heightIndex < 0
			|| sourceXIndex<0 || sourceYIndex<0 || theAttributes.count()!=6)
	{
		setErrorMessage("Invalid attributes for copy");
		return false;
	}
	else
	{
		int destinationX;
		if (!convertStringToInteger(theAttributes.value(destinationXIndex), destinationX))
		{
			setErrorMessage("Invalid destinationX " + theAttributes.value(destinationXIndex)+ " for copy");
			return false;
		}
		int destinationY;
		if (!convertStringToInteger(theAttributes.value(destinationYIndex), destinationY))
		{
			setErrorMessage("Invalid destinationY " + theAttributes.value(destinationYIndex)+ " for copy");
			return false;
		}
		int width;
		if (!convertStringToInteger(theAttributes.value(widthIndex), width))
		{
			setErrorMessage("Invalid witdh " + theAttributes.value(widthIndex)+ " for copy");
			return false;
		}
		int height;
		if (!convertStringToInteger(theAttributes.value(heightIndex), height))
		{
			setErrorMessage("Invalid height " + theAttributes.value(heightIndex)+ " for copy");
			return false;
		}
		int sourceX;
		if (!convertStringToInteger(theAttributes.value(sourceXIndex), sourceX))
		{
			setErrorMessage("Invalid sourceX " + theAttributes.value(sourceXIndex)+ " for copy");
			return false;
		}
		int sourceY;
		if (!convertStringToInteger(theAttributes.value(sourceYIndex), sourceY))
		{
			setErrorMessage("Invalid sourceY " + theAttributes.value(sourceYIndex)+ " for copy");
			return false;
		}

		QRect source(sourceX, sourceY, width, height);
		QPoint destination(destinationX, destinationY);
		CopyPainter* copyPainter=new CopyPainter(source, destination);
		currentDotPainter->addLCDPainter(copyPainter);

		return true;
	}
}

QtSkinException::QtSkinException(const QString& anErrorMessage)
: errorMessage(anErrorMessage)
{
}

QtSkinException::~QtSkinException() throw()
{
}

const char* QtSkinException::what() const throw()
{
	return errorMessage.toStdString().c_str();
}

TagHandler::TagHandler(StartElementMethod aStartElementMethod,
		EndElementMethod anEndElementMethod,
		CharactersMethod aCharactersMethod)
{
	startElementMethod=aStartElementMethod;
	endElementMethod=anEndElementMethod;
	charactersMethod=aCharactersMethod;
}

TagHandler::TagHandler()
{
	startElementMethod=NULL;
	endElementMethod=NULL;
	charactersMethod=NULL;
}

DocumentHandlers::DocumentHandlers()
{
	(*this)[QString("skin")]=TagHandler(&QtSkin::startSkin, NULL, NULL);
}

SkinHandlers::SkinHandlers()
{
	(*this)[QString("picture")]=TagHandler(&QtSkin::startPicture, NULL, NULL);
	(*this)[QString("size")]=TagHandler(&QtSkin::startSize, NULL, NULL);
	(*this)[QString("screen")]=TagHandler(&QtSkin::startScreen, NULL, NULL);
	(*this)[QString("foreground")]=TagHandler(&QtSkin::startForeground, NULL, NULL);
	(*this)[QString("background")]=TagHandler(&QtSkin::startBackground, NULL, NULL);
	(*this)[QString("hshift")]=TagHandler(&QtSkin::startHShift, NULL, NULL);
	(*this)[QString("keys")]=TagHandler(&QtSkin::startKeys, NULL, NULL);
	(*this)[QString("painters")]=TagHandler(&QtSkin::startPainters, NULL, NULL);
	(*this)[QString("pasters")]=TagHandler(&QtSkin::startPasters, NULL, NULL);
}

KeysHandlers::KeysHandlers()
{
	(*this)[QString("key")]=TagHandler(&QtSkin::startKey, NULL, NULL);
}

KeyHandlers::KeyHandlers()
{
	(*this)[QString("shortcut")]=TagHandler(&QtSkin::startShortcut, NULL, NULL);
}

PaintersHandlers::PaintersHandlers()
{
	(*this)[QString("painter")]=TagHandler(&QtSkin::startPainter, NULL, NULL);
}

PainterHandlers::PainterHandlers()
{
	(*this)[QString("polygon")]=TagHandler(NULL, NULL, &QtSkin::charactersPolygon);
	(*this)[QString("copy")]=TagHandler(&QtSkin::startCopy, NULL, NULL);
}

