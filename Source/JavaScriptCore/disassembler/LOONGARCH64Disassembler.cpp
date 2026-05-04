/*
 * Copyright (C) 2011-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2023-2024 Loongson Technology. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(DISASSEMBLER) && ENABLE(LOONGARCH64_DISASSEMBLER)

#include "MacroAssemblerCodeRef.h"

// lagoon is a small MIT-licensed LoongArch assembler/disassembler. Including
// the implementation here keeps JSC-only builds from needing CMake changes.
extern "C" {
#include "lagoon/lagoon.c"
}

namespace JSC {

bool tryToDisassemble(const CodePtr<DisassemblyPtrTag>& codePtr, size_t size, void*, void*, const char* prefix, PrintStream& out)
{
    uint32_t* currentPC = codePtr.untaggedPtr<uint32_t*>();
    size_t byteCount = size;

    while (byteCount) {
        lagoon_insn_t insn;
        la_disasm_one(*currentPC, &insn);

        char buffer[256];
        la_insn_to_str(&insn, buffer, sizeof(buffer));

        out.printf("%s%#16llx: <%08x> %s\n", prefix, static_cast<unsigned long long>(std::bit_cast<uintptr_t>(currentPC)), *currentPC, buffer);

        ++currentPC;
        byteCount -= sizeof(uint32_t);
    }

    return true;
}

} // namespace JSC

#endif // ENABLE(DISASSEMBLER) && ENABLE(LOONGARCH64_DISASSEMBLER)
