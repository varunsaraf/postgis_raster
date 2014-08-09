/***********************************************************************
 * File :    postgisrastertools.cpp
 * Project:  PostGIS Raster driver
 * Purpose:  Tools for PostGIS Raster driver
 * Author:   Jorge Arevalo, jorge.arevalo@deimos-space.com
 *                          jorgearevalo@libregis.org
 * 
 * Author:	 David Zwarg, dzwarg@azavea.com
 *
 * Last changes: $Id: $
 *
 ***********************************************************************
 * Copyright (c) 2009 - 2013, Jorge Arevalo, David Zwarg
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 **********************************************************************/
 #include "postgisraster.h"
 #include "cpl_conv.h"
 #include "cpl_error.h"
 #include "cpl_string.h" 
 /**********************************************************************
 * \brief Replace the quotes by single quotes in the input string
 *
 * Needed in the 'where' part of the input string
 **********************************************************************/
char * ReplaceQuotes(const char * pszInput, int nLength) {
    int i;
    char * pszOutput = NULL;

    if (nLength == -1)
        nLength = strlen(pszInput);

    pszOutput = (char*) CPLCalloc(nLength + 1, sizeof (char));

    for (i = 0; i < nLength; i++) {
        if (pszInput[i] == '"')
            pszOutput[i] = '\'';
        else
            pszOutput[i] = pszInput[i];
    }

    return pszOutput;
}

/**************************************************************
 * \brief Replace the single quotes by " in the input string
 *
 * Needed before tokenize function
 *************************************************************/
char * ReplaceSingleQuotes(const char * pszInput, int nLength) {
    int i;
    char* pszOutput = NULL;

    if (nLength == -1)
        nLength = strlen(pszInput);

    pszOutput = (char*) CPLCalloc(nLength + 1, sizeof (char));

    for (i = 0; i < nLength; i++) {
        if (pszInput[i] == '\'')
            pszOutput[i] = '"';
        else
            pszOutput[i] = pszInput[i];

    }

    return pszOutput;
}


/***********************************************************************
 * \brief Split connection string into user, password, host, database...
 *
 * The parameters separated by spaces are return as a list of strings.
 * The function accepts all the PostgreSQL recognized parameter keywords.
 *
 * The returned list must be freed with CSLDestroy when no longer needed
 **********************************************************************/
char** ParseConnectionString(const char * pszConnectionString) {
    char * pszEscapedConnectionString = NULL;

    /* Escape string following SQL scheme */
    pszEscapedConnectionString = 
        ReplaceSingleQuotes(pszConnectionString, -1);

    /* Avoid PG: part */
    char* pszStartPos = (char*) 
        strstr(pszEscapedConnectionString, ":") + 1;

    /* Tokenize */
    char** papszParams = 
        CSLTokenizeString2(pszStartPos, " ", CSLT_HONOURSTRINGS);

    /* Free */
    CPLFree(pszEscapedConnectionString);

    return papszParams;
}

/***********************************************************************
 * \brief Translate a PostGIS Raster datatype string in a valid 
 * GDALDataType object.
 **********************************************************************/
GBool TranslateDataType(const char * pszDataType,
        GDALDataType * poDataType = NULL, int * pnBitsDepth = NULL,
        GBool * pbSignedByte = NULL)
{
    if (!pszDataType)
        return false;

    if (pbSignedByte)
        *pbSignedByte = false;

    if (EQUALN(pszDataType, "1BB", 3 * sizeof(char))) {
        if (pnBitsDepth)
            *pnBitsDepth = 1;
        if (poDataType)
            *poDataType = GDT_Byte;
    }

    else if (EQUALN(pszDataType, "2BUI", 4 * sizeof (char))) {
        if (pnBitsDepth)
            *pnBitsDepth = 2;
        if (poDataType)
            *poDataType = GDT_Byte;
    }

    else if (EQUALN(pszDataType, "4BUI", 4 * sizeof (char))) {
        if (pnBitsDepth)
            *pnBitsDepth = 4;
        if (poDataType)
            *poDataType = GDT_Byte;
    }

    else if (EQUALN(pszDataType, "8BUI", 4 * sizeof (char))) {
        if (pnBitsDepth)
            *pnBitsDepth = 8;
        if (poDataType)
            *poDataType = GDT_Byte;
    }

    else if (EQUALN(pszDataType, "8BSI", 4 * sizeof (char))) {
        if (pnBitsDepth)
            *pnBitsDepth = 8;
        if (poDataType)
            *poDataType = GDT_Byte;

        /**
         * To indicate the unsigned byte values between 128 and 255
         * should be interpreted as being values between -128 and -1 for
         * applications that recognize the SIGNEDBYTE type.
         **/
        if (pbSignedByte)
            *pbSignedByte = true;
    }
    else if (EQUALN(pszDataType, "16BSI", 5 * sizeof (char))) {
        if (pnBitsDepth)
            *pnBitsDepth = 16;
        if (poDataType)
            *poDataType = GDT_Int16;
    }

    else if (EQUALN(pszDataType, "16BUI", 5 * sizeof (char))) {
        if (pnBitsDepth)
            *pnBitsDepth = 16;
        if (poDataType)
            *poDataType = GDT_UInt16;
    }

    else if (EQUALN(pszDataType, "32BSI", 5 * sizeof (char))) {
        if (pnBitsDepth)
            *pnBitsDepth = 32;
        if (poDataType)
            *poDataType = GDT_Int32;
    }

    else if (EQUALN(pszDataType, "32BUI", 5 * sizeof (char))) {
        if (pnBitsDepth)
            *pnBitsDepth = 32;
        if (poDataType)
            *poDataType = GDT_UInt32;
    }

    else if (EQUALN(pszDataType, "32BF", 4 * sizeof (char))) {
        if (pnBitsDepth)
            *pnBitsDepth = 32;
        if (poDataType)
            *poDataType = GDT_Float32;
    }

    else if (EQUALN(pszDataType, "64BF", 4 * sizeof (char))) {
        if (pnBitsDepth)
            *pnBitsDepth = 64;
        if (poDataType)
            *poDataType = GDT_Float64;
    }

    else {
        if (pnBitsDepth)
            *pnBitsDepth = -1;
        if (poDataType)
            *poDataType = GDT_Unknown;

        return false;
    }

    return true;
}

/***********************************************************************
 * \brief Translate a GDALDataType object to a valid PostGIS Raster
 * datatype string.
 **********************************************************************/
GBool TranslateDataTypeGDALtoPostGIS(char * pszDataType,
        GDALDataType * poDataType)
{
    if (!poDataType)
        return false;

    if (EQUAL((char *)poDataType, "GDT_Byte")) {
        pszDataType = CPLStrdup("8BUI");
    }

    else if (EQUAL((char *)poDataType, "GDT_Int16")) {
            pszDataType = CPLStrdup("16BSI");
    }

    else if (EQUAL((char *)poDataType, "GDT_UInt16")) {
            pszDataType = CPLStrdup("16BUI");
    }

    else if (EQUAL((char *)poDataType, "GDT_Int32")) {
            pszDataType = CPLStrdup("32BSI");
    }

    else if (EQUAL((char *)poDataType, "GDT_UInt32")) {
            pszDataType = CPLStrdup("32BUI");
    }

    else if (EQUAL((char *)poDataType, "GDT_Float32")) {
            pszDataType = CPLStrdup("32BF");
    }

    else if (EQUAL((char *)poDataType, "GDT_Float64")) {
            pszDataType = CPLStrdup("64BF");
    }

    else 
        return false;

    return true;
}

/*
int CPLStrlenUTF8(const char *pszUTF8Str) {
    int i = 0, j = 0;
    while (pszUTF8Str[i]) {
        if ((pszUTF8Str[i] & 0xc0) != 0x80) j++;
        i++;
    }
    return j;
}
*/

 /************************************************************************/
 /*                         EscapeString( )                         */
 /************************************************************************/

CPLString EscapeString(PGconn *hPGConn,
        const char* pszStrValue, int nMaxLength,
        const char* pszTableName,
        const char* pszFieldName )
{
    CPLString osCommand;

    /* We need to quote and escape string fields. */
    osCommand += "'";


    int nSrcLen = strlen(pszStrValue);
    int nSrcLenUTF = CPLStrlenUTF8(pszStrValue);

    if (nMaxLength > 0 && nSrcLenUTF > nMaxLength)
    {
        CPLDebug( "PG",
                "Truncated %s.%s field value '%s' to %d characters.",
                pszTableName, pszFieldName, pszStrValue, nMaxLength );
        nSrcLen = nSrcLen * nMaxLength / nSrcLenUTF;


        while( nSrcLen > 0 && ((unsigned char *) pszStrValue)[nSrcLen-1] > 127 )
        {
            CPLDebug( "PG", "Backup to start of multi-byte character." );
            nSrcLen--;
        }
    }

    char* pszDestStr = (char*)CPLMalloc(2 * nSrcLen + 1);

    /* -------------------------------------------------------------------- */
    /*  PQescapeStringConn was introduced in PostgreSQL security releases   */
    /*  8.1.4, 8.0.8, 7.4.13, 7.3.15                                        */
    /*  PG_HAS_PQESCAPESTRINGCONN is added by a test in 'configure'         */
    /*  so it is not set by default when building OGR for Win32             */
    /* -------------------------------------------------------------------- */
#if defined(PG_HAS_PQESCAPESTRINGCONN)
    int nError;
    PQescapeStringConn (hPGConn, pszDestStr, pszStrValue, nSrcLen, &nError);
    if (nError == 0)
        osCommand += pszDestStr;
    else
        CPLError(CE_Warning, CPLE_AppDefined, 
                "PQescapeString(): %s\n"
                "  input: '%s'\n"
                "    got: '%s'\n",
                PQerrorMessage( hPGConn ),
                pszStrValue, pszDestStr );
#else
    PQescapeString(pszDestStr, pszStrValue, nSrcLen);
    osCommand += pszDestStr;
#endif
    CPLFree(pszDestStr);

    osCommand += "'";

    return osCommand;
}
