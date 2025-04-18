/*
Copyright (c) 2013, iRacing.com Motorsport Simulations, LLC.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of iRacing.com Motorsport Simulations nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define MIN_WIN_VER 0x0501

#ifndef WINVER
#	define WINVER      MIN_WIN_VER
#endif

#ifndef _WIN32_WINNT
#	define _WIN32_WINNT    MIN_WIN_VER
#endif

#pragma warning(disable:4996) //_CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <cstdio>
#include <conio.h>
#include <ctime>


#include <IRacingSDK/LiveConnection.h>
#include <IRacingSDK/DiskClient.h>
#include <IRacingSDK/LiveClient.h>
#include <IRacingSDK/Types.h>
#include <IRacingSDK/Utils/YamlParser.h>


using namespace IRacingSDK;

void writeCSVSessionString(FILE *file, DiskSubHeader *diskSubHeader, const char *sessionInfoString) {
  if (file) {
    if (sessionInfoString) {
      // remove trailing ... from string
      int len = strlen(sessionInfoString);
      const char *pStr = strstr(sessionInfoString, "...");
      if (pStr)
        len = pStr - sessionInfoString;

      // and write the whole thing out
      fwrite(sessionInfoString, 1, len, file);
    } else {
      fprintf(file, "---\n");
    }

    if (diskSubHeader) {
      fprintf(file, "SessionLogInfo:\n");

      // get file open time as a string
      char tstr[512];
      tm tm_time;
      localtime_s(&tm_time, &diskSubHeader->startDate);
      strftime(tstr, 512, " %Y-%m-%d %H:%M:%S", &tm_time);
      tstr[512 - 1] = '\0';
      fprintf(file, " SessionStartDate: %s\n", tstr);

      fprintf(file, " SessionStartTime: %f\n", diskSubHeader->startTime);
      fprintf(file, " SessionEndTime: %f\n", diskSubHeader->endTime);
      fprintf(file, " SessionLapCount: %d\n", diskSubHeader->lapCount);
      fprintf(file, " SessionRecordCount: %d\n", diskSubHeader->sampleCount);
    }
    fprintf(file, "...\n");
  }
}

void writeCSVVarHeaders(FILE *file, DataHeader *header, VarDataHeader *varHeaders) {
  if (file && header && varHeaders) {
    int i, j;

    for (i = 0; i < header->numVars; i++) {
      int count = (varHeaders[i].type == VarDataType::Char) ? 1 : varHeaders[i].count;
      for (j = 0; j < count; j++) {
        if ((i + j) > 0)
          fputs(", ", file);

        if (count > 1)
          fprintf(file, "%s_%02d", varHeaders[i].name, j);
        else
          fputs(varHeaders[i].name, file);
      }
    }
    fputs("\n", file);

    for (i = 0; i < header->numVars; i++) {
      int count = (varHeaders[i].type == VarDataType::Char) ? 1 : varHeaders[i].count;
      for (j = 0; j < count; j++) {
        if ((i + j) > 0)
          fputs(", ", file);

        fputs(varHeaders[i].desc, file);
      }
    }
    fputs("\n", file);

    for (i = 0; i < header->numVars; i++) {
      int count = (varHeaders[i].type == VarDataType::Char) ? 1 : varHeaders[i].count;
      for (j = 0; j < count; j++) {
        if ((i + j) > 0)
          fputs(", ", file);

        fputs(varHeaders[i].unit, file);
      }
    }
    fputs("\n", file);

    for (i = 0; i < header->numVars; i++) {
      int count = (varHeaders[i].type == VarDataType::Char) ? 1 : varHeaders[i].count;
      for (j = 0; j < count; j++) {
        if ((i + j) > 0)
          fputs(", ", file);

        switch (varHeaders[i].type) {
          case VarDataType::Char:
            fputs("string", file);
            break;
          case VarDataType::Bool:
            fputs("boolean", file);
            break;
          case VarDataType::Int32:
            fputs("integer", file);
            break;
          case VarDataType::Bitmask:
            fputs("bitfield", file);
            break;
          case VarDataType::Float:
            fputs("float", file);
            break;
          case VarDataType::Double:
            fputs("double", file);
            break;
          default:
            fputs("unknown", file);
            break;
        }
      }
    }
    fputs("\n", file);
  }
}

void writeCSVData(FILE *file, DataHeader *header, VarDataHeader *varHeaders, char *lineBuf) {
  if (file && header && varHeaders && lineBuf) {
    int i, j;
    const VarDataHeader *rec;

    for (i = 0; i < header->numVars; i++) {
      rec = &varHeaders[i];
      //strings are an array of chars
      int count = (rec->type == VarDataType::Char) ? 1 : rec->count;

      for (j = 0; j < count; j++) {
        if ((i + j) > 0)
          fputs(", ", file);

        switch (rec->type) {
          case VarDataType::Char:
            fprintf(file, "%s", (char *) (lineBuf + rec->offset));
            break;
          case VarDataType::Bool:
            fprintf(file, "%d", ((bool *) (lineBuf + rec->offset))[j]);
            break;
          case VarDataType::Int32:
            fprintf(file, "%d", ((int *) (lineBuf + rec->offset))[j]);
            break;
          case VarDataType::Bitmask:
            fprintf(file, "%d", ((int *) (lineBuf + rec->offset))[j]);
            break;
          case VarDataType::Float:
            fprintf(file, "%g", ((float *) (lineBuf + rec->offset))[j]);
            break;
          case VarDataType::Double:
            fprintf(file, "%g", ((double *) (lineBuf + rec->offset))[j]);
            break;
          default:
            printf("found unknown type!\n");
            break;
        }
      }
    }
    fputs("\n", file);
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {

  }
  DataHeader header{};
  DiskSubHeader diskSubHeader{};

  char *sessionInfoString = nullptr;
  VarDataHeader *varHeaders = nullptr;
  char *varBuf = nullptr;
  size_t len;

  if (argc > 1) {
    char *fsrcName = argv[1];
    printf("opening %s\n", fsrcName);
    FILE *file = fopen(fsrcName, "rb");
    if (file) {
      len = sizeof(header);
      if (len == (int) fread(&header, 1, len, file)) {
        len = sizeof(diskSubHeader);
        if (len == (int) fread(&diskSubHeader, 1, len, file)) {
          sessionInfoString = new char[header.session.len];
          fseek(file, static_cast<long>(header.session.offset), SEEK_SET);
          len = header.session.len;
          if (len == fread(sessionInfoString, 1, len, file)) {
            sessionInfoString[header.session.len - 1] = '\0';

            varHeaders = new VarDataHeader[header.numVars];
            if (varHeaders) {
              fseek(file, header.varHeaderOffset, SEEK_SET);
              len = sizeof(VarDataHeader) * header.numVars;
              if (len == (int) fread(varHeaders, 1, len, file)) {
                varBuf = new char[header.bufLen];
                if (varBuf) {
                  fseek(file, header.varBuf[0].bufOffset, SEEK_SET);

                  // append .csv to file name
                  char *fname = new char[strlen(fsrcName) + 5];
                  strcpy(fname, fsrcName);
                  char *c = strrchr(fname, '.');
                  if (nullptr == c)
                    c = fname + strlen(fsrcName);
                  sprintf(c, ".csv");

                  FILE *fout = fopen(fname, "w");
                  if (fout) {
                    writeCSVSessionString(fout, &diskSubHeader, sessionInfoString);

                    writeCSVVarHeaders(fout, &header, varHeaders);

                    len = header.bufLen;
                    while (len == (int) fread(varBuf, 1, len, file))
                      writeCSVData(fout, &header, varHeaders, varBuf);

                    fclose(fout);
                  } else
                    printf("failed to open outfile %s\n", fname);
                  delete[]fname;
                  fname = nullptr;

                  if (varBuf)
                    delete[] varBuf;
                  varBuf = nullptr;
                } else
                  printf("failed to allocate varLineBuffer\n");
              } else
                printf("failed to read VarDataHeader\n");

              if (varHeaders)
                delete[]varHeaders;
              varHeaders = nullptr;
            } else
              printf("failed to allocate VarDataHeader\n");
          } else
            printf("failed to read sessionInfoString\n");

          if (sessionInfoString)
            delete[]sessionInfoString;
          sessionInfoString = nullptr;

        } else
          printf("error reading IRDiskSubHeader\n");
      } else
        printf("error reading DataHeader\n");

      fclose(file);
    } else
      printf("faild to open %s\n", fsrcName);
  } else
    printf("usage: irsdk_diskread <file.ibt>\n");

  return 0;
}

