// Scintilla source code edit control
/** @file LexACL.cxx
 ** Lexer for ACL
 **/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <windows.h>

#include "Scintilla.h"
#include "SciLexer.h"
#include "ILexer.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "WordList.h"
#include "LexerModule.h"
#include "StyleContext.h"

 /* ACL defines */
#define SCE_ACL_DEFAULT 0
#define SCE_ACL_COMMENT 1
#define SCE_ACL_COMMENTLINE 2
#define SCE_ACL_NUMBER 3
#define SCE_ACL_STRING 4
#define SCE_ACL_COMMAND 5
#define SCE_ACL_PARAMETER 6
#define SCE_ACL_FUNCTION 7
#define SCE_ACL_OPERATOR  8
#define SCE_ACL_IDENTIFIER 9
#define SCE_ACL_DELIMITER1 10
#define SCE_ACL_DELIMITER2 11
#define SCE_ACL_DELIMITER3 12
/* end ACL defines */
using namespace Lexilla;

bool noCharInRestLine(StyleContext &styleContex)
{
  int currentPosition = styleContex.currentPos;
  while( styleContex.More() )
  {
    int testChar = styleContex.GetRelative(0);
    if( (testChar == 0x0a) || (testChar == 0x0d) )
    {
      styleContex.currentPos = currentPosition;
      return true;
    }
    if( !isspace( testChar ) )
    {
      styleContex.currentPos = currentPosition;
      return false;
    }
    styleContex.currentPos++;
  };
  styleContex.currentPos = currentPosition;
  return true;
}

bool onlyLeadingSpaces(StyleContext &styleContex, unsigned int firstCharPos, unsigned int keyLen )
{
  int currentPosition = styleContex.currentPos;
  
  for( styleContex.currentPos = firstCharPos; styleContex.currentPos < currentPosition - keyLen; styleContex.currentPos++ )
  {
    int testChar = styleContex.GetRelative(0);
    if( !isspace( testChar ) )
    {
      styleContex.currentPos = currentPosition;
      return false;
    }
  };
  styleContex.currentPos = currentPosition;
  return true;
}

bool followedByChar(StyleContext &styleContex, const char* charList )
{
  int currentPosition = styleContex.currentPos;
  while( styleContex.More() )
  {
    int testChar = styleContex.GetRelative(0);
    if( isspace( testChar ) )
    {
      styleContex.currentPos++;
      continue;
    }
    for( int i=0; charList[i] != '\0'; i++ )
    {
      if( (testChar == charList[i]) )
      {
        styleContex.currentPos = currentPosition;
        return true;
      }
    }
    break;
  };
  styleContex.currentPos = currentPosition;
  return false;
}

bool isWhiteChar(unsigned char ch) 
{
	return (ch == ' ') || ((ch >= 0x09) && (ch <= 0x0d));
}

bool isAclChar(const int ch) 
{
	return (ch > 0x20) && (ch <= 0xFF);
}

bool isPrefixInListStartWith(WordList & list, const char *textToMatch, unsigned char firstChar) 
{
	int positionInList = list.starts[firstChar];

  int count = 0;
	if (positionInList >= 0)
	{
		while (toupper(list.words[positionInList][0]) == toupper(textToMatch[0]))
		{
			if (textToMatch[1] == '\0')
			{
				if (!list.words[positionInList][1])
        {
					return true;
				}
        else
        {
          count++;
          positionInList++;
          continue;
        }
			}
			int charInList = toupper(list.words[positionInList][1]);
			int charInToken = toupper(textToMatch[1]);
			if (charInList == charInToken) 
			{				
				const char *charPointerInList = list.words[positionInList] + 1;
				int upCaseCharInList = toupper((int)*charPointerInList);
				
				const char *charPointerInToken = textToMatch + 1;
				int upCaseCharInToken = toupper((int)*charPointerInToken);
			
				while (upCaseCharInToken && (upCaseCharInList == upCaseCharInToken))
				{
					charPointerInList++;
					upCaseCharInList = toupper((int)*charPointerInList);
					charPointerInToken++;
					upCaseCharInToken = toupper((int)*charPointerInToken);
				}
				if ((upCaseCharInList == 0) && (upCaseCharInToken == 0))
        {
					return true;
        }				
        else if( upCaseCharInToken == 0 )
        {
          count++;
          positionInList++;
          continue;
        }
			}
			positionInList++;
		}// while
  }// if( positionInList>=0 )
  if(count == 1 )
  {
	 size_t length = min(5, strlen(textToMatch));
     if(_strnicmp("scree", textToMatch, length ) != 0 ) 
     {
       return true;
     }
  }
  if(( count > 1)   && ((_strcmpi(textToMatch, "as") == 0) || 
                        (_strcmpi(textToMatch, "db") == 0) || 
                        (_strnicmp(textToMatch, "fie", 3) == 0) || 
                        (_strnicmp(textToMatch, "sup", 3) == 0) ||
                        (_strnicmp(textToMatch, "sep", 3) == 0) ||
                        (_strnicmp(textToMatch, "col", 3) == 0) ||
                        (_strnicmp(textToMatch, "rep", 3) == 0) ||
                        (_strnicmp(textToMatch, "tab", 3) == 0) ||
                        (_strnicmp(textToMatch, "fil", 3) == 0) ||
                        (_strnicmp(textToMatch, "inter", 3) == 0)) )
  {
    return true;
  }
  return false;
}

bool isPrefixNoCaseInList(WordList & list, const char *textToMatch) 
{
	if (list.words == NULL)
		return false;

	unsigned char firstChar = textToMatch[0];
	if( isPrefixInListStartWith(list, textToMatch, firstChar ) )
  {
    return true;
  }
 
  int value = ((unsigned char)textToMatch[0]); 
	if (!isalpha(value)) 
		return false;

	firstChar = isupper(value) ? tolower(value) : toupper(value);
	return isPrefixInListStartWith(list, textToMatch, firstChar );
}

bool isAclSymbol(char charToMatch)
{
  const char *symbols = "-!()*,`+<=>;";
  size_t sizeSymbols = strlen(symbols);
  for (unsigned int i = 0 ; i < sizeSymbols; i++)
  {
		if (charToMatch == symbols[i])
    {
			return true;
    }
  }
	return false;
}

bool isPrefixComment(char *textToMatch)
{
  size_t tokenLen = strlen(textToMatch);
  return (tokenLen >= 3) && (tokenLen <= 7) && 
         (_strnicmp(textToMatch, "comment", tokenLen) == 0);
}

void ColouriseAclDoc(unsigned int startPos, int length, int initStyle, WordList *keywordlists[], Accessor &styler) 
{
	WordList &commandKeywords   = *keywordlists[0];
	WordList &parameterKeywords = *keywordlists[1];
  WordList &functionKeywords  = *keywordlists[2];
  char aclDelimit[3] = { '"', '\'', '\0'};

	int previousNonWhiteChar = ' ';
	StyleContext styleContex(startPos, length, initStyle, styler);
  bool firstLineKeyword = true;
  bool firstCommandKeyword = true;
  unsigned int firstCharPos = 0;
  bool hadFunction = false;
	for (; styleContex.More(); styleContex.Forward()) 
	{
    if( styleContex.atLineStart )
    {
      firstLineKeyword = true;
      firstCommandKeyword = true;
      firstCharPos = styleContex.currentPos;
      hadFunction = false;
    }

    switch (styleContex.state)
		{			
			case SCE_ACL_DELIMITER1 :
			{
				if (aclDelimit[0] && (styleContex.ch == aclDelimit[0]) )
        {
          styleContex.ForwardSetState(SCE_ACL_DEFAULT);
        }
				break;
			}

			case SCE_ACL_DELIMITER2 :
			{
				if (aclDelimit[0] && (styleContex.ch == aclDelimit[1]) )
        {
          styleContex.ForwardSetState(SCE_ACL_DEFAULT);
        }
				break;
			}

			case SCE_ACL_DELIMITER3 :
			{
				if (aclDelimit[0] && (styleContex.ch == aclDelimit[2]))
        {
          styleContex.ForwardSetState(SCE_ACL_DEFAULT);
        }
				break;
			}
			
			case SCE_ACL_IDENTIFIER : 
			{
				if (!isAclChar(styleContex.ch) || isAclSymbol(styleContex.ch) )
				{
					bool doDefault = true;
					const int tokenLen = 100;
					char nextToken[tokenLen];
					styleContex.GetCurrent(nextToken, sizeof(nextToken));
          bool isComment = isPrefixComment( nextToken);
					bool isCommand = isPrefixNoCaseInList(commandKeywords, nextToken);
					bool isParameter = isPrefixNoCaseInList(parameterKeywords, nextToken);
					bool isFunction = isPrefixNoCaseInList(functionKeywords, nextToken) && (styleContex.ch == '(');
          bool isIf = _strcmpi( nextToken, "if") == 0;
          bool isElse = _strcmpi( nextToken, "else") == 0;
          if (isFunction )
          {
						styleContex.ChangeState(SCE_ACL_FUNCTION);
            hadFunction = true;
          }					
          else if ( isCommand && firstCommandKeyword && ( !isIf || onlyLeadingSpaces(styleContex, firstCharPos, (unsigned int)strlen(nextToken)))) 
					{
            if( !hadFunction || !followedByChar( styleContex, ",;)") )
            {
					    styleContex.ChangeState(SCE_ACL_COMMAND);
              if( !isIf && !isElse )
              {
                firstCommandKeyword = false;
              }
            }
					}
					else if ( isParameter &&  !onlyLeadingSpaces(styleContex, firstCharPos, (unsigned int)strlen(nextToken)) )
          {
            if( !hadFunction || !followedByChar( styleContex, ",;)") )
            {
						  styleContex.ChangeState(SCE_ACL_PARAMETER);
            }
          }
          else 
          {
	  				if( firstLineKeyword && isComment )
	  				{
	  					doDefault = false;
	  				  if( noCharInRestLine(styleContex) )
	  				  {
	  				    styleContex.ChangeState(SCE_ACL_COMMENT);
	  				  }
	  				  else
	  				  {
	  						styleContex.ChangeState(SCE_ACL_COMMENTLINE);
	  				  }
            }            
          }

          if (doDefault)
          {
						styleContex.SetState(SCE_ACL_DEFAULT);
          }
          firstLineKeyword = false;
				}
				break;
			}
			
			case SCE_ACL_COMMENT :
			{
				const char *pCommentClose = "end";

        if (styleContex.MatchIgnoreCase(pCommentClose) && (previousNonWhiteChar == ' '))
        {
          while( !styleContex.atLineEnd )
          {
						styleContex.Forward();
          }

					styleContex.SetState(SCE_ACL_DEFAULT);
        }
        else if( styleContex.atLineEnd && ( styleContex.atLineStart || (previousNonWhiteChar == ' ')  ) )
				{
					styleContex.SetState(SCE_ACL_DEFAULT);
				}

				break;
			} 
			
			case SCE_ACL_COMMENTLINE :
			{
				if (styleContex.atLineEnd) 
				{
					styleContex.SetState(SCE_ACL_DEFAULT);
				}
				break;
			} 
			case SCE_ACL_OPERATOR :
			{
				styleContex.SetState(SCE_ACL_DEFAULT);
				break;
			} 
			
			default :
				break;
		}

		// Determine if a new state should be entered.
		if (styleContex.state == SCE_ACL_DEFAULT) 
		{
	    if (aclDelimit[0] && (styleContex.ch == aclDelimit[0]))
      {
				styleContex.SetState(SCE_ACL_DELIMITER1);
      }
			else if (aclDelimit[0] && (styleContex.ch == aclDelimit[1]))
      {
				styleContex.SetState(SCE_ACL_DELIMITER2);
      }
			else if (aclDelimit[0] && (styleContex.ch == aclDelimit[2]))
      {
				styleContex.SetState(SCE_ACL_DELIMITER3);
      }
			else if (isAclSymbol( styleContex.ch))
      {
				styleContex.SetState(SCE_ACL_OPERATOR);
      }
			else if (isAclChar(styleContex.ch)) 
      {
				styleContex.SetState(SCE_ACL_IDENTIFIER);
      }
		}

		if (styleContex.atLineEnd) 
		{
		   if (styleContex.state == SCE_ACL_COMMENTLINE)
       {
				 styleContex.SetState(SCE_ACL_DEFAULT);
       }
			previousNonWhiteChar = ' ';
		}

		if (!isWhiteChar(styleContex.ch)) 
    {
			previousNonWhiteChar = styleContex.ch;
		}		
	}
	styleContex.Complete();
}

//LexerModule lmACL(SCLEX_ACL, ColouriseAclDoc, "acl");
