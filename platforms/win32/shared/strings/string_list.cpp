#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\strings\string_list.h"

void PushStringNode(string_node **List, char *String)
{
    string_node *NewNode = (string_node *)malloc(sizeof(string_node));
    ZeroMemory(NewNode->String, MAX_STRING_LENGTH);
    StringCchCat(NewNode->String, ArrayCount(NewNode->String), String);
    NewNode->NextNode = *List;
    *List = NewNode;
}

void FreeStringList(string_node *RootNode)
{
    string_node *CurrentNode = RootNode;
    string_node *ChildNode;

    while (CurrentNode)
    {
        ChildNode = CurrentNode->NextNode;
        free(CurrentNode);
        CurrentNode = ChildNode;
    }
}

void FlattenStringList(string_node *ListNode, char *Output, u32 OutputSize)
{
    while (ListNode)
    {
        StringCchCatA(Output, OutputSize, ListNode->String);
        StringCchCatA(Output, OutputSize, " ");
        ListNode = ListNode->NextNode;
    }
}