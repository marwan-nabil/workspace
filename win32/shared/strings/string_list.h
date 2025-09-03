#pragma once

#define MAX_STRING_LENGTH 1024

struct string_node
{
    char String[MAX_STRING_LENGTH];
    string_node *NextNode;
};

void PushStringNode(string_node **List, char *String);
void FreeStringList(string_node *RootNode);
void FlattenStringList(string_node *ListNode, char *Output, u32 OutputSize);