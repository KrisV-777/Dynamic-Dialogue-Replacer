ScriptName DynamicDialogueReplacer Hidden

; IMPORTANT: This script is a leftover of the old implementation and may need to be revisited at some point. Its usage is NOT recommended

; Replaces dialogue topics of any response branching from aiTopicFormId with asReplacementText
; Returns a unique identifier key to remove the replacement with RemoveReplacementTopic
; Only one replacement per topic can be active at a time, calling this again replaces the previous replacement and returns the new key
String Function AddReplacementTopic(int aiTopicFormId, string asReplacementText) native global

; Removes the replacement topic with the given key for aiTopicFormId, if the key is valid and matches the current active replacement
Function RemoveReplacementTopic(int aiTopicFormId, string asKey) native global
