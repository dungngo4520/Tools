#include <Windows.h>

#include <iostream>
#include <iomanip>
#include <memory>
#include <algorithm>

using namespace std;

typedef NTSTATUS(WINAPI* pRtlDecompressBuffer)(
    USHORT CompressionFormatAndEngine,
    PUCHAR UncompressedBuffer,
    ULONG UncompressedBufferSize,
    PUCHAR CompressedBuffer,
    ULONG CompressedBufferSize,
    PULONG FinalUncompressedSize
);

typedef NTSTATUS(WINAPI* pRtlDecompressFragment)(
    USHORT CompressionFormat,
    PUCHAR UncompressedFragment,
    ULONG UncompressedFragmentSize,
    PUCHAR CompressedBuffer,
    ULONG CompressedBufferSize,
    ULONG FragmentOffset,
    PULONG FinalUncompressedSize,
    PVOID WorkSpace
);

typedef NTSTATUS(WINAPI* pRtlGetCompressionWorkSpaceSize)(
    USHORT CompressionFormatAndEngine,
    PULONG CompressBufferWorkSpaceSize,
    PULONG CompressFragmentWorkSpaceSize
);

typedef NTSTATUS(WINAPI* pRtlDecompressFragmentEx)(
    USHORT CompressionFormat,
    PUCHAR UncompressedFragment,
    ULONG UncompressedFragmentSize,
    PUCHAR CompressedBuffer,
    ULONG CompressedBufferSize,
    ULONG FragmentOffset,
    ULONG UncompressedChunkSize,
    PULONG FinalUncompressedSize,
    PVOID WorkSpace
);

typedef union _COMPRESSED_CHUNK_HEADER {
    struct {
        USHORT CompressedChunkSizeMinus3 : 12;
        USHORT UncompressedChunkSize : 2;
        USHORT sbz : 1;
        USHORT IsChunkCompressed : 1;

    } Chunk;

    USHORT Short;

} COMPRESSED_CHUNK_HEADER, *PCOMPRESSED_CHUNK_HEADER;


#define MAGIC_STRING L"AJIANT"

BOOL MoveBackToLastChunk(HANDLE hFile, PVOID Buffer, SIZE_T BufferLength, PULONG64 MovedDistance)
{
    ULONG FragmentsTotalSize = 0;
    LARGE_INTEGER MoveDistance = {};

    COMPRESSED_CHUNK_HEADER* header = (COMPRESSED_CHUNK_HEADER*)Buffer;
    while (FragmentsTotalSize + header->Chunk.CompressedChunkSizeMinus3 + 3 <= BufferLength) {
        FragmentsTotalSize += header->Chunk.CompressedChunkSizeMinus3 + 3;
        header = (COMPRESSED_CHUNK_HEADER*)((PUCHAR)header + header->Chunk.CompressedChunkSizeMinus3 + 3);
    }

    MoveDistance.QuadPart = BufferLength - FragmentsTotalSize;
    MoveDistance.QuadPart = -MoveDistance.QuadPart;
    auto ret = SetFilePointerEx(hFile, MoveDistance, NULL, FILE_CURRENT);
    if (ret && MovedDistance) *MovedDistance = BufferLength - FragmentsTotalSize;
    return ret;
}

int wmain(int argc, wchar_t** argv)
{
    auto m = GetModuleHandleW(L"ntdll");
    auto RtlDecompressBuffer = (pRtlDecompressBuffer)GetProcAddress(m, "RtlDecompressBuffer");
    auto RtlDecompressFragment = (pRtlDecompressFragment)GetProcAddress(m, "RtlDecompressFragment");
    auto RtlGetCompressionWorkSpaceSize =
        (pRtlGetCompressionWorkSpaceSize)GetProcAddress(m, "RtlGetCompressionWorkSpaceSize");
    //auto RtlDecompressFragmentEx = (pRtlDecompressFragmentEx)GetProcAddress(m, "RtlDecompressFragmentEx");

    if (!RtlDecompressBuffer || !RtlDecompressFragment || !RtlGetCompressionWorkSpaceSize) {
        cout << "failed to locate decompress fn" << endl;
        return 1;
    }

    if (argc < 3) {
        wcout << argv[0] << L": <file compressed> <output>" << endl;
        return 0;
    }

    ULONG WorkSpaceSize = 0;
    ULONG FragmentWorkSpaceSize = 0;
    if (RtlGetCompressionWorkSpaceSize(COMPRESSION_FORMAT_LZNT1, &WorkSpaceSize, &FragmentWorkSpaceSize) < 0) {
        cout << "Failed to get workspace size" << endl;
    }

    unique_ptr<UCHAR> FragmentWorkspace(new UCHAR[FragmentWorkSpaceSize]);

    HANDLE hFile =
        CreateFileW(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        cout << "Open file failed " << GetLastError() << endl;
        return 1;
    }

    LARGE_INTEGER fileSize = {};
    if (!GetFileSizeEx(hFile, &fileSize)) {
        cout << "get file size failed " << GetLastError() << endl;
        return 1;
    }

    if (fileSize.QuadPart < sizeof(MAGIC_STRING) + 8) {
        cout << "Invalid file size" << endl;
        return 1;
    }

    cout << "File size " << fileSize.QuadPart << endl;

    ULONG ByteRead = 0;
    LARGE_INTEGER MoveDistance = {};
    MoveDistance.QuadPart = sizeof(MAGIC_STRING) + 8;
    MoveDistance.QuadPart = -MoveDistance.QuadPart;

    if (SetFilePointer(hFile, MoveDistance.LowPart, &MoveDistance.HighPart, FILE_END) == INVALID_SET_FILE_POINTER) {
        cout << "Set file pointer failed" << endl;
        return 1;
    }

    ULONG64 OriginalDataSize = 0;
    if (!ReadFile(hFile, &OriginalDataSize, 8, &ByteRead, NULL)) {
        cout << "Read Failed " << GetLastError() << endl;
        return 1;
    }

    cout << "Original file size " << OriginalDataSize << endl;

    WCHAR MagicString[] = MAGIC_STRING;
    memset(MagicString, 0, sizeof(MAGIC_STRING));

    if (!ReadFile(hFile, MagicString, sizeof(MAGIC_STRING), &ByteRead, NULL)) {
        cout << "Read Failed " << GetLastError() << endl;
        return 1;
    }

    if (memcmp(MagicString, MAGIC_STRING, sizeof(MAGIC_STRING)) != 0) {
        cout << "Invalid Buffer" << endl;
        return 1;
    }

    MoveDistance = {};
    if (!SetFilePointerEx(hFile, MoveDistance, NULL, FILE_BEGIN)) {
        cout << "Set file pointer failed " << GetLastError() << endl;
        return 1;
    }

    bool FileCreated = false;
    HANDLE hDes = INVALID_HANDLE_VALUE;
    if (argc >= 3) {
        hDes = CreateFileW(argv[2], GENERIC_WRITE | DELETE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hDes == INVALID_HANDLE_VALUE) {
            cout << "Failed to create des file " << GetLastError() << endl;
            return 1;
        }
        FileCreated = true;
    }

    bool Success = false;
    ULONG DecompressBufferSize = 64 * 1024;          // Buffer to store decompressed data
    ULONG CompressBufferReadSize = 4 * 1024 * 1024;  // Read 4MB in one go
    unique_ptr<UCHAR> DecompressBuffer(new UCHAR[DecompressBufferSize]);
    unique_ptr<UCHAR> CompressBuffer(new UCHAR[CompressBufferReadSize]);
    DWORD FileDataLeft = (DWORD)(fileSize.QuadPart - sizeof(MAGIC_STRING) - 8);
    ULONG64 TotalDecompressedSize = 0;  // Used to verify file

    while (ReadFile(hFile, CompressBuffer.get(), min(CompressBufferReadSize, FileDataLeft), &ByteRead, NULL)) {
        cout << "Processing " << fixed << showpoint << setprecision(2)
             << (double)TotalDecompressedSize / OriginalDataSize * 100 << "%" << endl;
        if (ByteRead == 0) {
            break;
        }
        FileDataLeft -= ByteRead;

        LONGLONG CompressedChunkProcessedTotalSize = 0;
        COMPRESSED_CHUNK_HEADER* header = (COMPRESSED_CHUNK_HEADER*)CompressBuffer.get();

        while (true) {
            auto UncompressChunkSize = 0;
            auto CompressedChunkSize = header->Chunk.CompressedChunkSizeMinus3 + 3;
            if (header->Chunk.IsChunkCompressed) {
                ULONG FinalSize = 0;
                auto status = RtlDecompressFragment(
                    COMPRESSION_FORMAT_LZNT1,
                    DecompressBuffer.get(),
                    DecompressBufferSize,
                    (PUCHAR)header,
                    CompressedChunkSize,
                    0,
                    &FinalSize,
                    FragmentWorkspace.get()
                );
                if (status >= 0) {
                    UncompressChunkSize = FinalSize;
                    cout << "Buffer is compressed, decompress size " << UncompressChunkSize << endl;
                }
                else {
                    // Decompress chunk failed, something is wrong with buffer
                    goto Exit;
                }
            }
            else {
                UncompressChunkSize = 1 << (header->Chunk.UncompressedChunkSize + 9);
                cout << "Buffer is not compressed, size " << UncompressChunkSize << endl;
                RtlCopyMemory(
                    DecompressBuffer.get(), (PUCHAR)header + sizeof(COMPRESSED_CHUNK_HEADER), UncompressChunkSize
                );
            }

            ULONG ByteWritten = 0;
            if (!WriteFile(hDes, DecompressBuffer.get(), UncompressChunkSize, &ByteWritten, NULL)) {
                cout << "Failed to write des file " << GetLastError() << endl;
                goto Exit;
            }
            TotalDecompressedSize += UncompressChunkSize;

            CompressedChunkProcessedTotalSize += CompressedChunkSize;
            header = (COMPRESSED_CHUNK_HEADER*)((PUCHAR)header + CompressedChunkSize);
            if (CompressedChunkProcessedTotalSize == ByteRead) {
                // Decompressed all chunk
                break;
            }
            else if (CompressedChunkProcessedTotalSize + header->Chunk.CompressedChunkSizeMinus3 + 3 > ByteRead) {
                // Next chunk does not sit entirely in buffer, move pointer at next chunk
                LARGE_INTEGER Move = {};
                Move.QuadPart = ByteRead - CompressedChunkProcessedTotalSize;
                Move.QuadPart = -Move.QuadPart;
                if (SetFilePointerEx(hFile, Move, NULL, FILE_CURRENT)) {
                    FileDataLeft += (DWORD)(ByteRead - CompressedChunkProcessedTotalSize);
                    break;
                }
                else {
                    cout << "Move pointer failed " << GetLastError() << endl;
                    goto Exit;
                }
            }
        }
    }

Exit:
    Success = TotalDecompressedSize == OriginalDataSize;
    cout << (Success ? "Decompress success" : "Decompress failed") << endl;
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    if (hDes != INVALID_HANDLE_VALUE) CloseHandle(hDes);
    if (!Success && FileCreated) {
        DeleteFileW(argv[2]);
    }
    return 0;
}