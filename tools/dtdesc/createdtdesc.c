/*
    Copyright (C) 2000-2020, The AROS Development Team. All rights reserved.

    Desc: DataTypesDescriptorCreator
*/

#include "createdtdesc.h"
#include "parser.h"

/****i* createdtdesc/main ***************************************************
*
*   NAME
*       main -- Run the DataType descriptor creator.
*
*   FUNCTION
*       Initializes descriptor state from the command line, parses the input and
*       requests descriptor output when initialization succeeds, then releases
*       the descriptor state.
*
*   INPUTS
*       argc - Command argument count supplied by the host C runtime.
*       argv - Command argument vector supplied by the host C runtime.
*
*   RESULT
*       0.
*
*   NOTES
*       Initialization and descriptor-write failures are not propagated through
*       the process return value by the current implementation.
*
****************************************************************************
*
*/

int main(int argc, char **argv)
{
 struct DTDesc *TheDTDesc;

 if(Init(argc, argv, &TheDTDesc))
 {
  Work(TheDTDesc);
 }

 Cleanup(TheDTDesc);

 return(0);
}

/****i* createdtdesc/HandleName *********************************************
*
*   NAME
*       HandleName -- Store the Name field from the current input line.
*
*   FUNCTION
*       Copies the data after Name= into the descriptor Name buffer and makes
*       that buffer the dth_Name string.
*
*   INPUTS
*       TheDTDesc - Descriptor state whose ReadBuffer contains the input line.
*
*   RESULT
*       TRUE on success; FALSE when TheDTDesc is NULL.
*
****************************************************************************
*
*/

int HandleName(struct DTDesc *TheDTDesc)
{
 uint8_t *DataPtr;

 if(!TheDTDesc)
 {
  return(FALSE);
 }

 TheDTDesc->ReadBuffer[READBUFFERSIZE-1]='\0';

 DataPtr=TheDTDesc->ReadBuffer+KeywordLength[Name];

 /*
  *  What, who yells buffer-overflow here?
  */

 strcpy(TheDTDesc->Name, DataPtr);

 TheDTDesc->DTH.dth_Name=TheDTDesc->Name;

 return(TRUE);
}

/****i* createdtdesc/HandleVersion ******************************************
*
*   NAME
*       HandleVersion -- Store the Version field from the current input line.
*
*   FUNCTION
*       Copies the data after Version= into the descriptor Version buffer.
*
*   INPUTS
*       TheDTDesc - Descriptor state whose ReadBuffer contains the input line.
*
*   RESULT
*       TRUE on success; FALSE when TheDTDesc is NULL.
*
****************************************************************************
*
*/

int HandleVersion(struct DTDesc *TheDTDesc)
{
 uint8_t *DataPtr;

 if(!TheDTDesc)
 {
  return(FALSE);
 }

 TheDTDesc->ReadBuffer[READBUFFERSIZE-1]='\0';

 DataPtr=TheDTDesc->ReadBuffer+KeywordLength[Version];

 /*
  *  What, who yells buffer-overflow here?
  */

 strcpy(TheDTDesc->Version, DataPtr);

 return(TRUE);
}

/****i* createdtdesc/HandleBaseName *****************************************
*
*   NAME
*       HandleBaseName -- Store the BaseName field from the current input line.
*
*   FUNCTION
*       Copies the data after BaseName= into the descriptor BaseName buffer and
*       makes that buffer the dth_BaseName string.
*
*   INPUTS
*       TheDTDesc - Descriptor state whose ReadBuffer contains the input line.
*
*   RESULT
*       TRUE on success; FALSE when TheDTDesc is NULL.
*
****************************************************************************
*
*/

int HandleBaseName(struct DTDesc *TheDTDesc)
{
 uint8_t *DataPtr;

 if(!TheDTDesc)
 {
  return(FALSE);
 }

 TheDTDesc->ReadBuffer[READBUFFERSIZE-1]='\0';

 DataPtr=TheDTDesc->ReadBuffer+KeywordLength[BaseName];

 /*
  *  What, who yells buffer-overflow here?
  */

 strcpy(TheDTDesc->BaseName, DataPtr);

 TheDTDesc->DTH.dth_BaseName=TheDTDesc->BaseName;

 return(TRUE);
}

/****i* createdtdesc/HandlePattern ******************************************
*
*   NAME
*       HandlePattern -- Store the Pattern field from the current input line.
*
*   FUNCTION
*       Copies the data after Pattern= into the descriptor Pattern buffer and
*       makes that buffer the dth_Pattern string.
*
*   INPUTS
*       TheDTDesc - Descriptor state whose ReadBuffer contains the input line.
*
*   RESULT
*       TRUE on success; FALSE when TheDTDesc is NULL.
*
****************************************************************************
*
*/

int HandlePattern(struct DTDesc *TheDTDesc)
{
 uint8_t *DataPtr;

 if(!TheDTDesc)
 {
  return(FALSE);
 }

 TheDTDesc->ReadBuffer[READBUFFERSIZE-1]='\0';

 DataPtr=TheDTDesc->ReadBuffer+KeywordLength[Pattern];

 /*
  *  What, who yells buffer-overflow here?
  */

 strcpy(TheDTDesc->Pattern, DataPtr);

 TheDTDesc->DTH.dth_Pattern=TheDTDesc->Pattern;

 return(TRUE);
}

/****i* createdtdesc/HandleMask *********************************************
*
*   NAME
*       HandleMask -- Parse the recognition mask from the current input line.
*
*   FUNCTION
*       Parses quoted characters, ANY wildcards, and numeric values after
*       Mask=. Whitespace is skipped. Parsed values are appended to the Mask
*       buffer and dth_MaskLen is updated; dth_Mask is then set to that buffer.
*
*   INPUTS
*       TheDTDesc - Descriptor state whose ReadBuffer contains the input line.
*
*   RESULT
*       TRUE on success; FALSE when TheDTDesc is NULL.
*
*   NOTES
*       Unrecognized mask text is skipped rather than reported as an error.
*
****************************************************************************
*
*/

int HandleMask(struct DTDesc *TheDTDesc)
{
 uint8_t *DataPtr;
 int i;

 if(!TheDTDesc)
 {
  return(FALSE);
 }

 TheDTDesc->ReadBuffer[READBUFFERSIZE-1]='\0';

 DataPtr=TheDTDesc->ReadBuffer+KeywordLength[Mask];

#if 1
 {
  uint8_t *NewDataPtr;
  int done = 0;
  
  TheDTDesc->DTH.dth_MaskLen = 0;
  
  while(!done)
  {
   uint8_t c = *DataPtr++;

   switch(c)
   {
    case '\0':
     done = 1;
     break;
     
    case '\'':
     c = *DataPtr++;
     TheDTDesc->Mask[TheDTDesc->DTH.dth_MaskLen++] = c;
     if (c) c = *DataPtr++;
     if (!c) done = 1;
     break;
     
    case 'A':
     c = *DataPtr++;
     if (!c) done = 1;
     if (c != 'N') break;
     c = *DataPtr++;
     if (!c) done = 1;
     if (c != 'Y') break;
     TheDTDesc->Mask[TheDTDesc->DTH.dth_MaskLen++] = 0xFFFF;
     break;
     
    case ' ':
    case '\t':
     break;
     
    default:
     DataPtr--;
     i = strtol(DataPtr, (char **)(&NewDataPtr), 0);
     if (DataPtr != NewDataPtr)
     {
      DataPtr = NewDataPtr;
      TheDTDesc->Mask[TheDTDesc->DTH.dth_MaskLen++] = i;
     }
     else
     {
      DataPtr++;
     }
     break;
     
   }
  }
 }
 
#else
 TheDTDesc->DTH.dth_MaskLen=(uint16_t) strlen(DataPtr);
 if(!TheDTDesc->DTH.dth_MaskLen)
 {
  return(TRUE);
 }

 for(i=0; i<TheDTDesc->DTH.dth_MaskLen; i++)
 {
  TheDTDesc->Mask[i] = (DataPtr[i]==0xFF) ? 0xFFFF : (uint16_t) DataPtr[i];
 }
#endif

 TheDTDesc->DTH.dth_Mask=TheDTDesc->Mask;

 return(TRUE);
}

/****i* createdtdesc/HandleGroupID ******************************************
*
*   NAME
*       HandleGroupID -- Store a DataType group identifier.
*
*   FUNCTION
*       Uses the first four bytes after GroupID= to construct dth_GroupID. An
*       empty field explicitly selects group ID zero; non-empty group IDs must
*       contain at least four bytes.
*
*   INPUTS
*       TheDTDesc - Descriptor state whose ReadBuffer contains the input line.
*
*   RESULT
*       TRUE for an empty field or when at least four data bytes are present.
*       FALSE when TheDTDesc is NULL or a non-empty field is shorter than four
*       bytes.
*
****************************************************************************
*
*/

int HandleGroupID(struct DTDesc *TheDTDesc)
{
 const uint8_t *DataPtr;

 if(!TheDTDesc)
 {
  return(FALSE);
 }

 TheDTDesc->ReadBuffer[READBUFFERSIZE-1]='\0';

 DataPtr=TheDTDesc->ReadBuffer+KeywordLength[GroupID];

 if(!DataPtr[0])
 {
  TheDTDesc->DTH.dth_GroupID=0;
  return(TRUE);
 }

 if(!(DataPtr[1] && DataPtr[2] && DataPtr[3]))
 {
  return(FALSE);
 }

 TheDTDesc->DTH.dth_GroupID=MAKE_ID(DataPtr[0], DataPtr[1], DataPtr[2], DataPtr[3]);

 return(TRUE);
}

/****i* createdtdesc/HandleID ***********************************************
*
*   NAME
*       HandleID -- Store a DataType identifier of up to four characters.
*
*   FUNCTION
*       Uses the first four bytes after ID= to construct dth_ID and pads a
*       shorter non-empty identifier with zero bytes, matching AROS IDs such
*       as ID_IFF. Additional bytes after the first four are ignored.
*
*   INPUTS
*       TheDTDesc - Descriptor state whose ReadBuffer contains the input line.
*
*   RESULT
*       TRUE when at least one data byte is present. FALSE when TheDTDesc is
*       NULL or the field is empty.
*
****************************************************************************
*
*/

int HandleID(struct DTDesc *TheDTDesc)
{
 const uint8_t *DataPtr;
 uint8_t IDBytes[4] = { 0, 0, 0, 0 };
 int i;

 if(!TheDTDesc)
 {
  return(FALSE);
 }

 TheDTDesc->ReadBuffer[READBUFFERSIZE-1]='\0';

 DataPtr=TheDTDesc->ReadBuffer+KeywordLength[ID];

 if(!DataPtr[0])
 {
  return(FALSE);
 }

 for(i=0; (i<4) && DataPtr[i]; i++)
 {
  IDBytes[i]=DataPtr[i];
 }

 TheDTDesc->DTH.dth_ID=MAKE_ID(IDBytes[0], IDBytes[1], IDBytes[2], IDBytes[3]);

 return(TRUE);
}

/****i* createdtdesc/HandleFlags ********************************************
*
*   NAME
*       HandleFlags -- Parse supported DataType flags from the current line.
*
*   FUNCTION
*       Clears dth_Flags, scans the data after Flags= for the supported flag
*       names, and ORs the corresponding values into dth_Flags. Other text is
*       skipped.
*
*   INPUTS
*       TheDTDesc - Descriptor state whose ReadBuffer contains the input line.
*
*   RESULT
*       TRUE on success; FALSE when TheDTDesc is NULL.
*
****************************************************************************
*
*/

int HandleFlags(struct DTDesc *TheDTDesc)
{
 uint8_t *DataPtr;
 long Len;
 int i;

 const char *TheFlags[] =
 {
  "DTF_BINARY",
  "DTF_ASCII",
  "DTF_IFF",
  "DTF_MISC",
  "DTF_CASE",
  "DTF_SYSTEM1"
 };

 const int FlagLength[] =
 {
  10,
  9,
  7,
  8,
  8,
  11
 };

 const int NumFlags=6;

 const uint16_t FlagValues[] =
 {
  0x0000,
  0x0001,
  0x0002,
  0x0003,
  0x0010,
  0x1000
 };

 if(!TheDTDesc)
 {
  return(FALSE);
 }

 TheDTDesc->ReadBuffer[READBUFFERSIZE-1]='\0';

 DataPtr=TheDTDesc->ReadBuffer+KeywordLength[Flags];

 TheDTDesc->DTH.dth_Flags=0;

 Len=strlen(DataPtr);
 if(!Len)
 {
  return(TRUE);
 }

 while(Len>0)
 {
  for(i=0; i<NumFlags; i++)
  {
   if(Len<FlagLength[i])
   {
    continue;
   }

   if(strncmp(TheFlags[i], DataPtr, FlagLength[i])==0)
   {
    TheDTDesc->DTH.dth_Flags |= FlagValues[i];

    Len-=(FlagLength[i]-1);
    DataPtr+=(FlagLength[i]-1);

    break;
   }
  }

  Len--;
  DataPtr++;
 }

 return(TRUE);
}

/****i* createdtdesc/HandlePriority *****************************************
*
*   NAME
*       HandlePriority -- Store the decimal DataType priority.
*
*   FUNCTION
*       Parses the data after Priority= with strtoul() in base 10 and stores
*       the result in the 16-bit dth_Priority field.
*
*   INPUTS
*       TheDTDesc - Descriptor state whose ReadBuffer contains the input line.
*
*   RESULT
*       TRUE on success; FALSE when TheDTDesc is NULL.
*
*   NOTES
*       The parsed value is not range-validated before conversion to uint16_t.
*
****************************************************************************
*
*/

int HandlePriority(struct DTDesc *TheDTDesc)
{
 uint8_t *DataPtr;
 unsigned long Pri;

 if(!TheDTDesc)
 {
  return(FALSE);
 }

 TheDTDesc->ReadBuffer[READBUFFERSIZE-1]='\0';

 DataPtr=TheDTDesc->ReadBuffer+KeywordLength[Priority];

 Pri=strtoul(DataPtr, NULL, 10);

 TheDTDesc->DTH.dth_Priority=(uint16_t) Pri;

 return(TRUE);
}

/****i* createdtdesc/HandleLine *********************************************
*
*   NAME
*       HandleLine -- Dispatch one DataTypeDescription input line.
*
*   FUNCTION
*       Compares ReadBuffer with the known Keyword= prefixes and calls the
*       corresponding field handler for the first match. Lines with no known
*       prefix are ignored.
*
*   INPUTS
*       TheDTDesc - Descriptor state containing the current input line.
*
*   RESULT
*       The matched handler result, TRUE when no keyword matched, or FALSE when
*       TheDTDesc is NULL.
*
****************************************************************************
*
*/

int HandleLine(struct DTDesc *TheDTDesc)
{
 int RetVal;
 int i;

 if(!TheDTDesc)
 {
  return(FALSE);
 }

 RetVal=TRUE;

 for(i=0; i<NumKeywords; i++)
 {
  if(strncmp(TheDTDesc->ReadBuffer, Keywords[i], KeywordLength[i])==0)
  {
   RetVal=KeywordHandler[i](TheDTDesc);

   break;
  }
 }

 return(RetVal);
}

/****i* createdtdesc/RemoveNewLine ******************************************
*
*   NAME
*       RemoveNewLine -- Remove the trailing newline from an input line.
*
*   FUNCTION
*       Replaces a final '\n' in ReadBuffer with a NUL character.
*
*   INPUTS
*       TheDTDesc - Descriptor state containing a NUL-terminated line read by
*                   Work().
*
*   RESULT
*       TRUE after processing; FALSE when TheDTDesc is NULL.
*
*   NOTES
*       A carriage return is not removed.
*
****************************************************************************
*
*/

int RemoveNewLine(struct DTDesc *TheDTDesc)
{
 int Len;

 if(!TheDTDesc)
 {
  return(FALSE);
 }

 Len=strlen(TheDTDesc->ReadBuffer);

 if(TheDTDesc->ReadBuffer[Len-1]=='\n')
 {
  TheDTDesc->ReadBuffer[Len-1]='\0';
 }

 return(TRUE);
}

/****i* createdtdesc/Work ***************************************************
*
*   NAME
*       Work -- Parse the input stream and request descriptor output.
*
*   FUNCTION
*       Reads lines into ReadBuffer, removes the trailing newline, and dispatches
*       each line until end of input or a field handler fails. WriteOutDTD() is
*       then called once.
*
*   INPUTS
*       TheDTDesc - Initialized descriptor state and input stream.
*
*   NOTES
*       The return values from RemoveNewLine() and WriteOutDTD() are not
*       propagated by this void function.
*
****************************************************************************
*
*/

void Work(struct DTDesc *TheDTDesc)
{

 if(!TheDTDesc)
 {
  return;
 }

 memset(TheDTDesc->ReadBuffer, '\0', READBUFFERSIZE);

 while(fgets(TheDTDesc->ReadBuffer, READBUFFERSIZE, TheDTDesc->Input))
 {
  RemoveNewLine(TheDTDesc);

  if(!HandleLine(TheDTDesc))
  {
   break;
  }

  memset(TheDTDesc->ReadBuffer, '\0', READBUFFERSIZE);
 }

 WriteOutDTD(TheDTDesc);

}

/****i* createdtdesc/WriteOutDTD ********************************************
*
*   NAME
*       WriteOutDTD -- Write the parsed descriptor as an IFF FORM DTYP file.
*
*   FUNCTION
*       Requires non-empty Name and BaseName fields, supplies #? when Pattern is
*       empty, and writes NAME, optional FVER, DTHD, and optional DTCD chunks to
*       OutputName. DTHD numeric fields and mask words are converted to the file
*       byte order used by the descriptor.
*
*   INPUTS
*       TheDTDesc - Parsed descriptor state and output pathname.
*
*   RESULT
*       TRUE when the descriptor is written; FALSE on validation, open, chunk,
*       or data-write failure.
*
*   NOTES
*       The mask words in TheDTDesc are byte-swapped in place when required.
*       Detected output-write failures remove the invocation-owned OutputName.
*       NewIFF() owns cleanup if its initial FORM header cannot be written.
*
****************************************************************************
*
*/

int WriteOutDTD(struct DTDesc *TheDTDesc)
{
 struct IFFHandle *IH;
 struct FileDataTypeHeader FileDTH;
 int i;

 if(!TheDTDesc)
 {
  return(FALSE);
 }

 if(strlen(TheDTDesc->Name)==0)
 {
  return(FALSE);
 }

 if(strlen(TheDTDesc->BaseName)==0)
 {
  return(FALSE);
 }

#if 0
 if(TheDTDesc->DTH.dth_MaskLen==0)
 {
  return(FALSE);
 }
#endif

 if(strlen(TheDTDesc->Pattern)==0)
 {
  TheDTDesc->Pattern[0]='#';
  TheDTDesc->Pattern[1]='?';
  TheDTDesc->Pattern[2]='\0';
 }

 IH=NewIFF(TheDTDesc->OutputName, MAKE_ID('D','T','Y','P'));
 if(!IH)
 {
  return(FALSE);
 }

 if(!NewChunk(IH, MAKE_ID('N','A','M','E')))
 {
  CloseIFF(IH);
  remove(TheDTDesc->OutputName);
  return(FALSE);
 }

 if(WriteChunkData(IH, TheDTDesc->Name, (strlen(TheDTDesc->Name)+1))<=0)
 {
  EndChunk(IH);
  CloseIFF(IH);
  remove(TheDTDesc->OutputName);
  return(FALSE);
 }

 EndChunk(IH);

 if(strlen(TheDTDesc->Version) > 0)
 {
  if(!NewChunk(IH, MAKE_ID('F','V','E','R')))
  {
   CloseIFF(IH);
   remove(TheDTDesc->OutputName);
   return(FALSE);
  }

  if(WriteChunkData(IH, TheDTDesc->Version, (strlen(TheDTDesc->Version)+1))<=0)
  {
   EndChunk(IH);
   CloseIFF(IH);
   remove(TheDTDesc->OutputName);
   return(FALSE);
  }

  EndChunk(IH);
 }

 if(!NewChunk(IH, MAKE_ID('D','T','H','D')))
 {
  CloseIFF(IH);
  remove(TheDTDesc->OutputName);
  return(FALSE);
 }

 //ASSERT((sizeof(struct FileDataTypeHeader) & 1) == 0);
 if (TheDTDesc->DTH.dth_MaskLen > 0)
 {
  // Write the mask directly after the header to preserve its word alignment.
  FileDTH.dth_Mask     = (((unsigned int) sizeof(struct FileDataTypeHeader)));
  FileDTH.dth_Name     = (((unsigned int) FileDTH.dth_Mask) + (TheDTDesc->DTH.dth_MaskLen << 1));
  FileDTH.dth_Mask     = Swap32IfLE(((uint32_t) FileDTH.dth_Mask));
 }
 else
 {
  FileDTH.dth_Mask     = 0;
  FileDTH.dth_Name     = (((unsigned int) sizeof(struct FileDataTypeHeader)));
 }
 FileDTH.dth_BaseName = (((unsigned int) FileDTH.dth_Name) + strlen(TheDTDesc->DTH.dth_Name) + 1);
 FileDTH.dth_Pattern  = (((unsigned int) FileDTH.dth_BaseName) + strlen(TheDTDesc->DTH.dth_BaseName) + 1);
 FileDTH.dth_GroupID  = TheDTDesc->DTH.dth_GroupID;
 FileDTH.dth_ID       = TheDTDesc->DTH.dth_ID;
 FileDTH.dth_MaskLen  = TheDTDesc->DTH.dth_MaskLen;
 FileDTH.dth_Pad      = TheDTDesc->DTH.dth_Pad;
 FileDTH.dth_Flags    = TheDTDesc->DTH.dth_Flags;
 FileDTH.dth_Priority = TheDTDesc->DTH.dth_Priority;

 FileDTH.dth_Name     = Swap32IfLE(((uint32_t) FileDTH.dth_Name));
 FileDTH.dth_BaseName = Swap32IfLE(((uint32_t) FileDTH.dth_BaseName));
 FileDTH.dth_Pattern  = Swap32IfLE(((uint32_t) FileDTH.dth_Pattern));
 FileDTH.dth_GroupID  = Swap32IfLE(FileDTH.dth_GroupID);
 FileDTH.dth_ID       = Swap32IfLE(FileDTH.dth_ID);
 FileDTH.dth_MaskLen  = Swap16IfLE(FileDTH.dth_MaskLen);
 FileDTH.dth_Pad      = Swap16IfLE(FileDTH.dth_Pad);
 FileDTH.dth_Flags    = Swap16IfLE(FileDTH.dth_Flags);
 FileDTH.dth_Priority = Swap16IfLE(FileDTH.dth_Priority);

 if(WriteChunkData(IH, (char *) &FileDTH, sizeof(struct FileDataTypeHeader))<=0)
 {
  EndChunk(IH);
  CloseIFF(IH);
  remove(TheDTDesc->OutputName);
  return(FALSE);
 }

 if (TheDTDesc->DTH.dth_MaskLen)
 {
  for(i=0; i<TheDTDesc->DTH.dth_MaskLen; i++)
  {
   TheDTDesc->DTH.dth_Mask[i]=Swap16IfLE(TheDTDesc->DTH.dth_Mask[i]);
  }

  if(WriteChunkData(IH, (char *) TheDTDesc->DTH.dth_Mask, TheDTDesc->DTH.dth_MaskLen << 1)<=0)
  {
   EndChunk(IH);
   CloseIFF(IH);
   remove(TheDTDesc->OutputName);
   return(FALSE);
  }
 }

 if(WriteChunkData(IH, TheDTDesc->DTH.dth_Name, (strlen(TheDTDesc->DTH.dth_Name) + 1))<=0)
 {
  EndChunk(IH);
  CloseIFF(IH);
  remove(TheDTDesc->OutputName);
  return(FALSE);
 }

 if(WriteChunkData(IH, TheDTDesc->DTH.dth_BaseName, (strlen(TheDTDesc->DTH.dth_BaseName) + 1))<=0)
 {
  EndChunk(IH);
  CloseIFF(IH);
  remove(TheDTDesc->OutputName);
  return(FALSE);
 }

 if(WriteChunkData(IH, TheDTDesc->DTH.dth_Pattern, (strlen(TheDTDesc->DTH.dth_Pattern) + 1))<=0)
 {
  EndChunk(IH);
  CloseIFF(IH);
  remove(TheDTDesc->OutputName);
  return(FALSE);
 }
 
 EndChunk(IH);

 if(TheDTDesc->DTCD)
 {
  void *DTCDdata = NULL;
  int DTCDsize;
  FILE *dtcdInput=fopen(TheDTDesc->DTCD, "r");
  if(dtcdInput)
  {
   fseek(dtcdInput, 0, SEEK_END); // seek to end of file
   DTCDsize = ftell(dtcdInput); // get current file pointer
   fseek(dtcdInput, 0, SEEK_SET); // seek back to beginning of file
   DTCDdata=malloc(DTCDsize);
   if (fread(DTCDdata, DTCDsize, 1, dtcdInput) > 0)
   {
    // TODO: validate the data is HUNK
   }
   fclose(dtcdInput);
  }

  if (DTCDdata)
  {
   if(!NewChunk(IH, MAKE_ID('D','T','C','D')))
   {
    CloseIFF(IH);
    remove(TheDTDesc->OutputName);
    return(FALSE);
   }

   if(WriteChunkData(IH, DTCDdata, DTCDsize)<=0)
   {
    free(DTCDdata);
    EndChunk(IH);
    CloseIFF(IH);
    remove(TheDTDesc->OutputName);
    return(FALSE);
   }
   free(DTCDdata);
   EndChunk(IH);
  }
 }
 CloseIFF(IH);

 return(TRUE);
}

/****i* createdtdesc/Init ***************************************************
*
*   NAME
*       Init -- Allocate and initialize descriptor-creator state.
*
*   FUNCTION
*       Allocates a zeroed DTDesc, records the program name, selects stdin and
*       the Name buffer as the initial input and output choices, parses command
*       arguments, and opens the selected input file.
*
*   INPUTS
*       argc      - Command argument count.
*       argv      - Command argument vector; must not be NULL.
*       TheDTDesc - Receives the allocated descriptor state; must not be NULL.
*
*   RESULT
*       TRUE when argument parsing and input opening succeed; FALSE otherwise.
*       After allocation, *TheDTDesc remains available for Cleanup() on failure.
*
****************************************************************************
*
*/

int Init(int argc, char **argv, struct DTDesc **TheDTDesc)
{
 struct DTDesc *NewDTDesc;

 if(!(argv && TheDTDesc))
 {
  return(FALSE);
 }

 *TheDTDesc=NULL;

 NewDTDesc=(struct DTDesc *) malloc(sizeof(struct DTDesc));
 if(!NewDTDesc)
 {
  return(FALSE);
 }

 *TheDTDesc=NewDTDesc;

 memset(NewDTDesc, '\0', sizeof(struct DTDesc));

 NewDTDesc->ProgName=argv[0];
 NewDTDesc->Input=stdin;
 NewDTDesc->OutputName=NewDTDesc->Name;

 if(!ParseArgs(argc, argv, NewDTDesc))
 {
  return(FALSE);
 }

 if(!OpenInput(NewDTDesc))
 {
  return(FALSE);
 }

 return(TRUE);
}

/****i* createdtdesc/OpenInput **********************************************
*
*   NAME
*       OpenInput -- Open the selected DataTypeDescription input file.
*
*   FUNCTION
*       Opens InputName for text input when a name was supplied. If no input
*       name was supplied, the existing stdin selection is retained.
*
*   INPUTS
*       TheDTDesc - Descriptor state containing InputName and Input.
*
*   RESULT
*       TRUE when no file is required or the file opens successfully. FALSE for
*       a NULL state or a failed fopen(); after an open failure Input is reset
*       to stdin.
*
****************************************************************************
*
*/

int OpenInput(struct DTDesc *TheDTDesc)
{

 if(!TheDTDesc)
 {
  return(FALSE);
 }

 if(TheDTDesc->InputName)
 {
  TheDTDesc->Input=fopen(TheDTDesc->InputName, "r");
  if(!TheDTDesc->Input)
  {
   TheDTDesc->Input=stdin;

   return(FALSE);
  }
 }

 return(TRUE);
}

/****i* createdtdesc/ParseArgs **********************************************
*
*   NAME
*       ParseArgs -- Parse createdtdesc command-line arguments.
*
*   FUNCTION
*       Processes -o and -m option arguments, displays usage for -h, and stores
*       other arguments as InputName. A later non-option argument replaces an
*       earlier InputName.
*
*   INPUTS
*       argc      - Command argument count.
*       argv      - Command argument vector; must not be NULL.
*       TheDTDesc - Descriptor state to update; must not be NULL.
*
*   RESULT
*       TRUE after parsing; FALSE for invalid pointers, a missing -o/-m value,
*       or -h.
*
****************************************************************************
*
*/

int ParseArgs(int argc, char **argv, struct DTDesc *TheDTDesc)
{
 int i;

 if(!(argv && TheDTDesc))
 {
  return(FALSE);
 }

 for(i=1; i<argc; i++)
 {
  if(strcmp(argv[i], "-o") == 0)
  {
   if(++i >= argc)
   {
    Usage(TheDTDesc->ProgName);

    return(FALSE);
   }

   TheDTDesc->OutputName=argv[i];
  }
  else if(strcmp(argv[i], "-m") == 0)
  {
   if(++i >= argc)
   {
    Usage(TheDTDesc->ProgName);

    return(FALSE);
   }
   TheDTDesc->DTCD=argv[i];
  }
  else if(strcmp(argv[i], "-h") == 0)
  {
   Usage(TheDTDesc->ProgName);

   return(FALSE);
  }
  else
  {
   TheDTDesc->InputName=argv[i];
  }
 }

 return(TRUE);
}

/****i* createdtdesc/Usage **************************************************
*
*   NAME
*       Usage -- Print the createdtdesc command syntax.
*
*   FUNCTION
*       Writes the usage line to stderr, using the supplied program name or
*       createdtdesc when ProgName is NULL.
*
*   INPUTS
*       ProgName - Program name to display, or NULL for the default name.
*
****************************************************************************
*
*/

void Usage(char *ProgName)
{
 char DefaultName[]="createdtdesc";
 char *NamePtr;

 NamePtr = ProgName ? ProgName : DefaultName;

 fprintf(stderr, "\n"
                 "usage: %s [-o <Output-Name>] [-m <DTCD binary>] <Input-Name>\n"
                 "\n",
        NamePtr);
}

/****i* createdtdesc/Cleanup ************************************************
*
*   NAME
*       Cleanup -- Release descriptor-creator state.
*
*   FUNCTION
*       Closes an input stream owned by the descriptor state when it is not
*       stdin, then frees the DTDesc allocation.
*
*   INPUTS
*       TheDTDesc - Descriptor state to release; NULL is accepted.
*
****************************************************************************
*
*/

void Cleanup(struct DTDesc *TheDTDesc)
{
 if(TheDTDesc)
 {
  if((TheDTDesc->Input!=NULL) && (TheDTDesc->Input!=stdin))
  {
   fclose(TheDTDesc->Input);
  }

  free((void *) TheDTDesc);
 }
}


