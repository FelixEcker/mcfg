/* test_parse.c ; mcfg
 * Used to test functionality and memory-safety of the mbcfg parsing
 * lib.
 */

#include <mcfg.h>
#include <stdlib.h>
#include <stdio.h>

void print_structure(struct mcfg_file* file) {
  printf("\n==========================\n\n");
  for (int i = 0; i < file->sector_count; i++) {
    mcfg_sector sector = file->sectors[i];
    printf("sector: %s\n", sector.name);
    printf("sections:\n");
    for (int j = 0; j < sector.section_count; j++) {
      mcfg_section section = sector.sections[j];
      printf("  section: %s\n", section.name);
      printf("  lines: %s\n", section.lines);
      printf("  fields:\n");
      for (int k = 0; k < section.field_count; k++) {
        mcfg_field field = section.fields[k];
        printf("    name: %s\n", field.name);
        printf("    value: %s\n", field.value);
      }
    }
  }
  printf("\n==========================\n");
}

int main() {
  struct mcfg_file* file = malloc(sizeof(mcfg_file));
  file->path = "./bugtest.mb";
  int result = parse_file(file);

  if ((result & MCFG_ERR_MASK_ERRNO) == MCFG_ERR_MASK_ERRNO)
    result = result ^ 0;

  if (result != 0) {
    printf("Parsing failed: Line %d\n", file->line);
    printf("Parsing failed: 0x%.8x\n", result);
  } else {
    print_structure(file);
		char *resolved = resolve_fields((*file),
              find_field(file, ".config/mariebuild/finalize_cmd")->value, 
              ".config/mariebuild/"
           );
    printf("%s\n", resolved);
		free(resolved);
  }

  free_mcfg_file(file);

  return result;
}
