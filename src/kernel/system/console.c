#include <kernel/utils/string.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/memory/malloc.h>
#include <kernel/fs/vfs.h>
#include <kernel/proc/task.h>
#include "psf.h"
#include "console.h"

#define VIDEO_VADDR 0xFC000000
#define TEXT_COLOR 0xFFFFFF
#define BACKGROUND_COLOR 0x000000

extern process *current_process;

static uint32_t current_column = 0;
static uint32_t current_row = 0;

static char *video_paddr;
static uint32_t video_bpp, video_pitch;
static uint32_t video_width, video_height;

void print_char(const char c)
{
  psf_putchar(c, current_column, current_row, TEXT_COLOR, BACKGROUND_COLOR, VIDEO_VADDR, video_pitch);

  if (current_column >= video_width)
  {
    current_column = 0;
    current_row += 1;
  }
  else
  {
    current_column += 1;
  }
}

void print_string(const char *s)
{
  psf_puts(s, current_column, current_row, TEXT_COLOR, BACKGROUND_COLOR, VIDEO_VADDR, video_pitch);

  uint32_t length = strlen(s);
  if (current_column + length >= video_width)
  {
    current_column = 0;
    current_row += 1;
  }
  else
  {
    current_column += length;
  }
}

int printf(const char *fmt, ...)
{
  if (!fmt)
    return 0;

  va_list args;
  va_start(args, fmt);

  size_t i = 0;
  size_t length = strlen(fmt);
  for (; i < length; i++)
  {
    switch (fmt[i])
    {
    case '%':
      switch (fmt[i + 1])
      {

      case 'c':
      {
        char c = va_arg(args, int);
        print_char(c);
        i++;
        break;
      }

      case 's':
      {
        char *c = va_arg(args, char *);
        char fmt[64];
        strcpy(fmt, (const char *)c);
        print_string(fmt);
        i++;
        break;
      }

      case 'd':
      case 'i':
      {
        int c = va_arg(args, int);
        char fmt[32] = {0};
        itoa_s(c, 10, fmt);
        print_string(fmt);
        i++;
        break;
      }

      case 'X':
      case 'x':
      {
        unsigned int c = va_arg(args, unsigned int);
        char fmt[32] = {0};
        itoa_s(c, 16, fmt);
        print_string(fmt);
        i++;
        break;
      }

      default:
        va_end(args);
        return 1;
      }

      break;

    default:
      print_char(fmt[i]);
      break;
    }
  }

  va_end(args);
  return 0;
}

void console_init(struct multiboot_tag_framebuffer *multiboot_framebuffer)
{
  video_paddr = multiboot_framebuffer->common.framebuffer_addr;
  video_bpp = multiboot_framebuffer->common.framebuffer_bpp;
  video_pitch = multiboot_framebuffer->common.framebuffer_pitch;
  video_width = multiboot_framebuffer->common.framebuffer_width;
  video_height = multiboot_framebuffer->common.framebuffer_height;

  uint32_t screen_size = video_height * video_pitch;
  uint32_t blocks = div_ceil(screen_size, PMM_FRAME_SIZE);
  for (uint32_t i = 0; i < blocks; ++i)
    vmm_map_address(
        vmm_get_directory(),
        VIDEO_VADDR + i * PMM_FRAME_SIZE,
        video_paddr + i * PMM_FRAME_SIZE,
        I86_PTE_PRESENT | I86_PTE_WRITABLE);
}

void console_setup()
{
  kstat *stat = malloc(sizeof(kstat));
  vfs_stat("/fonts/ter-powerline-v16n.psf", stat);
  char *buf = malloc(stat->size);
  long fd = vfs_open("/fonts/ter-powerline-v16n.psf");
  vfs_fread(fd, buf, stat->size);
  psf_init(buf, stat->size);
}