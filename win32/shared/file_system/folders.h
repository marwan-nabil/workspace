#pragma once

#include "portable\shared\base_types.h"

singly_linked_list_node *GetListOfFilesInFolder(char *DirectoryPath, char *Extension, linear_allocator *Allocator);