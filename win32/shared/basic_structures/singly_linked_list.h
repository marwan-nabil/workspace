#pragma once

#include <windows.h>
#include "portable\shared\base_types.h"
#include "win32\shared\memory\arena.h"

struct singly_linked_list_node
{
    void *Value;
    singly_linked_list_node *NextNode;
};

inline singly_linked_list_node *
GetLastNodeOfSinglyLinkedList(singly_linked_list_node *AnyListNode)
{
    Assert(AnyListNode);
    while (AnyListNode->NextNode)
    {
        AnyListNode = AnyListNode->NextNode;
    }
    return AnyListNode;
}

inline singly_linked_list_node *
HeadPushSinglyLinkedListNode
(
    singly_linked_list_node *ListHeadNode,
    void *NewNodeValue,
    memory_arena *ArenaAllocator
)
{
    singly_linked_list_node *NewNode = (singly_linked_list_node *)
        PushOntoMemoryArena(ArenaAllocator, sizeof(singly_linked_list_node), FALSE);
    NewNode->Value = NewNodeValue;
    NewNode->NextNode = ListHeadNode;
    return NewNode;
}

inline singly_linked_list_node *
TailPushSinglyLinkedListNode
(
    singly_linked_list_node *AnyListNode,
    void *NewNodeValue,
    memory_arena *ArenaAllocator
)
{
    singly_linked_list_node *NewNode = (singly_linked_list_node *)
        PushOntoMemoryArena(ArenaAllocator, sizeof(singly_linked_list_node), FALSE);
    NewNode->Value = NewNodeValue;
    NewNode->NextNode = NULL;

    if (AnyListNode)
    {
        singly_linked_list_node *LastNode = GetLastNodeOfSinglyLinkedList(AnyListNode);
        LastNode->NextNode = NewNode;
    }
    return NewNode;
}