// Copyright Natali Caggiano. All Rights Reserved.

/**
 * Unit tests for the Claude CLI silence watchdog logic.
 * Tests the pure formatter and latch transitions without spawning a real subprocess.
 */

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "ClaudeCodeRunner.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FClaudeCodeRunner_BuildHangDiagnostic_IncludesAllFields,
	"UnrealClaude.SilenceWatchdog.BuildHangDiagnostic.IncludesAllFields",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FClaudeCodeRunner_BuildHangDiagnostic_IncludesAllFields::RunTest(const FString& Parameters)
{
	const FString Payload = TEXT("{\"type\":\"user\",\"message\":{\"role\":\"user\",\"content\":\"hello\"}}");
	const FString Buffer = TEXT("partial-line-no-newline");

	const FString Diagnostic = FClaudeCodeRunner::BuildHangDiagnostic(
		/*SilenceSeconds*/ 62.5,
		/*bProcRunning*/ true,
		Payload,
		Buffer,
		/*Pending*/ 3,
		/*Running*/ 1,
		/*Completed*/ 7);

	TestTrue(TEXT("Contains silence seconds"), Diagnostic.Contains(TEXT("62")));
	TestTrue(TEXT("Contains proc-running status"), Diagnostic.Contains(TEXT("proc_running=true")));
	TestTrue(TEXT("Contains payload byte count"), Diagnostic.Contains(FString::Printf(TEXT("payload_bytes=%d"), Payload.Len())));
	TestTrue(TEXT("Contains MCP queue counts"), Diagnostic.Contains(TEXT("mcp_queue=3/1/7")));
	TestTrue(TEXT("Contains at least some payload content"), Diagnostic.Contains(TEXT("hello")));
	TestTrue(TEXT("Contains buffer preview"), Diagnostic.Contains(TEXT("partial-line-no-newline")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FClaudeCodeRunner_BuildHangDiagnostic_TruncatesLongPayload,
	"UnrealClaude.SilenceWatchdog.BuildHangDiagnostic.TruncatesLongPayload",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FClaudeCodeRunner_BuildHangDiagnostic_TruncatesLongPayload::RunTest(const FString& Parameters)
{
	// 2000-char payload with distinctive head and tail so we can prove both made it
	const FString Head = FString::ChrN(500, TEXT('H'));
	const FString Middle = FString::ChrN(1000, TEXT('M'));
	const FString Tail = FString::ChrN(500, TEXT('T'));
	const FString Payload = Head + Middle + Tail;

	const FString Diagnostic = FClaudeCodeRunner::BuildHangDiagnostic(
		61.0, true, Payload, TEXT(""), 0, 0, 0);

	TestTrue(TEXT("Contains first-500 chars (HHH...)"), Diagnostic.Contains(Head));
	TestTrue(TEXT("Contains last-500 chars (TTT...)"), Diagnostic.Contains(Tail));
	TestFalse(TEXT("Does NOT contain the middle 1000 chars (MMM...)"), Diagnostic.Contains(Middle));
	TestTrue(TEXT("Reports correct byte count"),
		Diagnostic.Contains(FString::Printf(TEXT("payload_bytes=%d"), Payload.Len())));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FClaudeCodeRunner_BuildHangDiagnostic_HandlesEmptyPayload,
	"UnrealClaude.SilenceWatchdog.BuildHangDiagnostic.HandlesEmptyPayload",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FClaudeCodeRunner_BuildHangDiagnostic_HandlesEmptyPayload::RunTest(const FString& Parameters)
{
	const FString Diagnostic = FClaudeCodeRunner::BuildHangDiagnostic(
		65.0, false, TEXT(""), TEXT(""), 0, 0, 0);

	TestTrue(TEXT("Empty payload byte count is 0"), Diagnostic.Contains(TEXT("payload_bytes=0")));
	TestTrue(TEXT("proc_running=false surfaced"), Diagnostic.Contains(TEXT("proc_running=false")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
