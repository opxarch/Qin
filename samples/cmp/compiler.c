/** @file
 * Qin - Samples compiler.
 */

/*
 *  Qin is Copyright (C) 2016, The 1st Middle School in
 *  Yongsheng Lijiang, Yunnan Province, ZIP 674200 China
 *
 *  This project is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License(GPL)
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *
 *  This project is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 */

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "wavetable/wavebank.h"

#define DEBUG 0

#define MAX_PATH_LEN (255)

#define CHUNK_SIZE (16384)

#define SAMPLE_TABLE_HEADER "QIN SAMPLE TABLE 1\n"

/*******************************************************************************
*   Typedefs & Structures                                                      *
*******************************************************************************/
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

/**
 * RIFF file header
 */
typedef struct __attribute__((__packed__)) RIFF_HEADER_s
{
  char      szRiffID[4];    // 'R','I','F','F'
  DWORD     dwRiffSize;     // the total size expect for the 'id' and 'size' fields.
  char      szRiffFormat[4]; // 'W','A','V','E'
} RIFF_HEADER_t;

/**
 * Wave format fields
 */
typedef struct __attribute__((__packed__)) WAVEFORMAT_s
{
  WORD      wFormatTag;
  WORD      nChannels;
  DWORD     nSamplesPerSec;
  DWORD     nAvgBytesPerSec;
  WORD      nBlockAlign;
} WAVEFORMAT_t;

typedef struct __attribute__((__packed__)) PCMWAVEFORMAT_s
{
  WAVEFORMAT_t  wf;
  WORD          wBitsPerSample;
} PCMWAVEFORMAT_t;

/*
 * format block field (only for PCMWAVE)
 */
typedef struct __attribute__((__packed__)) FMTBLOCK_s
{
  char          szFmtID[4]; // 'f','m','t',' '
  DWORD         dwFmtSize;
  PCMWAVEFORMAT_t wavFormat;
} FMTBLOCK_t;

/*
 * PCM data chunk
 */
typedef struct __attribute__((__packed__)) PCMDATA_s
{
  char          szDataID[4];
  DWORD         dwDataSize;
} PCMDATA_t;

/*
 * Fact chunk
 */
typedef struct __attribute__((__packed__)) FACTBLOCK_s
{
  char  szFactID[4]; // 'f','a','c','t'
  DWORD  dwFactSize;
} FACTBLOCK_t;

#define WAVE_FORMAT_PCM         (0x0001)
#define WAVE_FORMAT_IEEE_FLOAT  (0x0003)

////////////////////////////////////////////////////////////////////////////////

/**
 * Print the usage of this compiler.
 * @return exit code.
 */
static
int
usage(const char *argv0)
{
  fprintf(stderr, "Sample compiler: Usage :\n");
  fprintf(stderr, "\t%s [input] [output] [table output] [bank] [dynamics] [note].\n", argv0);
  fprintf(stderr, "\t%s -ct [table output]      - Create a empty table file.\n");
  fprintf(stderr, "\t%s -cw [wave bank output]  - Create a empty wave bank file.\n");
  fflush(stderr);
  return 1;
}

/**
 * Print the error message.
 * @return exit code.
 */
static
int
cmperr(const char *msg)
{
  fprintf(stderr, "error: %s\n", msg);
  fflush(stderr);
  return 1;
}

/**
 * Main entry of compiler.
 */
int
main(int argc, char *argv[])
{
  FILE *inf;
  FILE *outf;
  FILE *tablef;

  char *srcname[MAX_PATH_LEN+1];
  const char *rawfile;
  const char *bank;
  const char *dynamics;
  const char *note;

  size_t len = 0, total = 0, required = 0;
  size_t wlen = 0;
  RIFF_HEADER_t bufRiff;
  FMTBLOCK_t bufFmt;
  FACTBLOCK_t bufFact;
  PCMDATA_t bufData;
  char buff[CHUNK_SIZE];

  /*
   * If we have reviced a -c parameter, then
   * we should set about creating the sample table file.
   */
  if (argc == 3 && strncmp(argv[1], "-ct", 3)==0)
    {
      tablef = fopen(argv[2], "wb");
      if (!tablef)
          return cmperr("failed on creating the table.");
      len = fprintf(tablef, SAMPLE_TABLE_HEADER);
      if (len<=0 || ferror(tablef))
          return cmperr("failed on writing the table.");
      fclose(tablef);
      /*
       * The procedure of compilation will end here.
       */
      return 0;
    }
  else if (argc == 3 && strncmp(argv[1], "-cw", 3)==0)
    {
      FILE *wf = fopen(argv[2], "wb");
      if (!wf)
          return cmperr("failed on creating the wave bank.");

      WaveBank_t waveHdr;
      memset(&waveHdr, 0, sizeof(waveHdr));
      waveHdr.magic[0] = 'Q';
      waveHdr.magic[1] = 'W';
      waveHdr.magic[2] = 'S';
      waveHdr.magic[3] = 'F';

      len = fwrite(&waveHdr, sizeof(waveHdr), 1, wf);
      if (len<=0 || ferror(wf))
          return cmperr("failed on writing the wave bank.");
      fclose(wf);
      /*
       * The procedure of compilation will end here.
       */
      return 0;
    }
  else if (argc < 7)
    {
      /* invalid usage of this compiler */
      return usage(argv[0]);
    }

  rawfile   = argv[2];
  bank      = argv[4];
  dynamics  = argv[5];
  note      = argv[6];
  memset(srcname, 0, sizeof(srcname));

  /*
   * Get the base name
   */
  len = strchr(argv[1], '.') ? (size_t)(strchr(argv[1], '.') - argv[1])
      : strlen(argv[1]);
  len = len < MAX_PATH_LEN ? len : MAX_PATH_LEN;
  memcpy(srcname, argv[1], len);

  /*
   * Open files
   */
  inf = fopen(argv[1], "rb");
  outf = fopen(argv[2], "ab");
  tablef = fopen(argv[3], "ab");

  if (!inf)
    {
      return cmperr("failed on opening the input file.");
    }
  if (!outf)
    {
      return cmperr("failed on creating the output file.");
    }
  if (!tablef)
    {
      return cmperr("failed on creating the table output.");
    }

  /*
   * Read the RIFF header
   */
  len = fread((void*)(&bufRiff), sizeof(bufRiff), 1, inf);
  if (len != 1)
    {
      return cmperr("failed on reading the input.");
    }

#if DEBUG
  printf("RIFF header:\n");
  printf("\tmagic = %.4s\n", bufRiff.szRiffID);
  printf("\tformat = %.4s\n", bufRiff.szRiffFormat);
#endif

  if (bufRiff.szRiffID[0] != 'R' ||
      bufRiff.szRiffID[1] != 'I' ||
      bufRiff.szRiffID[2] != 'F' ||
      bufRiff.szRiffID[3] != 'F')
    {
      return cmperr("invalid magic number of the input.");
    }

  if (bufRiff.szRiffFormat[0] != 'W' ||
      bufRiff.szRiffFormat[1] != 'A' ||
      bufRiff.szRiffFormat[2] != 'V' ||
      bufRiff.szRiffFormat[3] != 'E')
    {
      return cmperr("invalid format of the input.");
    }

  /*
   * Read the fmt block
   */
  len = fread((void*)(&bufFmt), sizeof(bufFmt), 1, inf);
  if (len != 1)
    {
      return cmperr("failed on reading the input.");
    }

#if DEBUG
  printf("\nFmt header:\n");
  printf("\tmagic = %.4s\n", bufFmt.szFmtID);
  printf("\tfmtsize = 0x%x\n", bufFmt.dwFmtSize);
#endif

  if (bufFmt.szFmtID[0] != 'f' ||
      bufFmt.szFmtID[1] != 'm' ||
      bufFmt.szFmtID[2] != 't' ||
      bufFmt.szFmtID[3] != ' ')
    {
      return cmperr("invalid magic number of fmt block.");
    }

  if (bufFmt.dwFmtSize != sizeof(PCMWAVEFORMAT_t))
    {
      return cmperr("the data of input file may be dameged.");
    }
  if (bufFmt.wavFormat.wf.wFormatTag != WAVE_FORMAT_PCM)
    {
      return cmperr("the format of input is not a PCM Wave file.");
    }

  /*
   * Read the wave format fields
   */
#if DEBUG
  printf("\nWAVE format:\n");
  printf("\tchannels = %d\n", bufFmt.wavFormat.wf.nChannels);
  printf("\tbps = %d bits/sec\n", bufFmt.wavFormat.wBitsPerSample);
  printf("\tfs = %d Hz\n", bufFmt.wavFormat.wf.nSamplesPerSec);
  printf("\ttag = 0x%x\n", bufFmt.wavFormat.wf.wFormatTag);
  printf("\talign = %d\n", bufFmt.wavFormat.wf.nBlockAlign);
#endif

  /*
   * Read the wave data chunk
   */
  len = fread((void*)(&bufData), sizeof(bufData), 1, inf);
  if (len != 1)
    {
      return cmperr("failed on reading the input.");
    }

#if DEBUG
  printf("\nWAVE data chunk:\n");
  printf("\tmagic = %.4s\n", bufData.szDataID);
  printf("\tsize = %d\n", bufData.dwDataSize);
#endif

  if (bufData.szDataID[0] != 'd' ||
      bufData.szDataID[1] != 'a' ||
      bufData.szDataID[2] != 't' ||
      bufData.szDataID[3] != 'a')
    {
      return cmperr("the data of input file may be dameged.");
    }

  /*
   * There may be a fact chunk followed the data chunk,
   * so we should deal with it.
   */
  if (bufData.dwDataSize == 0)
    {
      len = fread((void*)(&bufFact), sizeof(bufFact), 1, inf);
      if (len != 1)
        {
          return cmperr("failed on reading the input.");
        }

      if (bufFact.szFactID[0] != 'f' ||
          bufFact.szFactID[1] != 'a' ||
          bufFact.szFactID[2] != 'c' ||
          bufFact.szFactID[0] != 't')
        {
          /*
           * Not a fact chunk, so we shoul decide what to do
           * in this case.
           */
          return cmperr("the length of the sounds is abnormally zero.");
        }

#if DEBUG
      printf("\nFact chunk(detected)\n");
      printf("\tsize = %d\n", bufFact.dwFactSize);
#endif

      //!todo
      return -1;
    }

  /*
   * Read the PCM data
   */
  required = bufData.dwDataSize;
  total = 0;
  for (;;)
    {
      len = fread(buff, 1, sizeof(buff), inf);
      if (len)
        {
          total += len;
          if (total > required)
            {
              /*
               * the number of bytes read is out of the value indicated by the input file.
               */
              return cmperr("internal error.");
            }

          /*
           * Write the pcm data to the target
           */
          wlen = fwrite(buff, 1, len, outf);
          if (wlen <=0 || ferror(outf) )
            {
              return cmperr("failed on writing the output file.");
            }
          continue;
        }
      else if (ferror(inf))
        {
          return cmperr("failed on reading the input.");
        }
      else
        break;
    }

  if (total != required)
    {
      return cmperr("the input file was damaged.");
    }

  /*
   * Append the wave table
   */
  len = fprintf(tablef, "%s %s %s %s %s %d %d %d %d %d %d \n",
                note,
                srcname,
                rawfile,
                bank,
                dynamics,
                required,
                bufFmt.wavFormat.wf.nChannels,
                bufFmt.wavFormat.wBitsPerSample,
                bufFmt.wavFormat.wf.nSamplesPerSec,
                bufFmt.wavFormat.wf.wFormatTag,
                bufFmt.wavFormat.wf.nBlockAlign);
  if (len <=0 || ferror(tablef))
    {
      return cmperr("failed on writing table output.");
    }

  fclose(outf);
  fclose(inf);
  fclose(tablef);

  return 0;
}
