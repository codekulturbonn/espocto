/**
 * Convert a .ch8 file to a .ec8 file.
 * .ec8 files are used by the emulator. They contain an octo_option structure,
 * followed by the raw data of the .ch8 file. 
 */
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include "../vendor/c-octo/src/octo_emulator.h"

int
main(int argc, char *argv[])
{
  FILE* f;
  char *p, *base, *ec8_filename;
  char* buffer;
  int size;
  octo_options options;

  if (argc != 4) {
    fprintf(stderr, "%d Usage: %s .../input.ch8 shiftQuirks loadStoreQuirks\n", argc, argv[0]);
    return 1;
  }

  if (strcmp(argv[1] + strlen(argv[1]) - 4, ".ch8") != 0) {
    fprintf(stderr, "Error: Input file must have .ch8 extension\n");
    return 1;
  }

  base = strdup(argv[1]);
  p = strrchr(base, '/');
  if (p) {
    ec8_filename = p + 1;
  }
  else {
    ec8_filename = base;
  }
  strcpy(ec8_filename + strlen(ec8_filename) - 4, ".ec8");
  
  f = fopen(argv[1], "rb");
  if (f == NULL) {
    fprintf(stderr, "Error: Could not read file %s\n", argv[1]);
    return 1;
  }
  fseek(f, 0, SEEK_END);
  size = ftell(f);
  rewind(f); 

  octo_default_options(&options);
  options.q_shift = strcmp(argv[2], "1") == 0;
  options.q_loadstore = strcmp(argv[3], "1") == 0;

  buffer = malloc(size + sizeof(octo_options));
  memcpy(buffer, &options, sizeof(octo_options));
  fread(buffer + sizeof(octo_options), 1, size, f);
  fclose(f);

  f = fopen(ec8_filename, "wb");
  if (f == NULL) {
    fprintf(stderr, "Error: Could not write file %s\n", ec8_filename);
    return 1;
  }
  fwrite(buffer, 1, size + sizeof(octo_options), f);
  fclose(f);

  free(buffer);
  free(base);
  return 0;
}
