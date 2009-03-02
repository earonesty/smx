/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

        3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "mapstr.h"

hash_val_t HashCaseCharPtr(const void *key)
  {
    static unsigned long randbox[] = {
        0x49848f1bU, 0xe6255dbaU, 0x36da5bdcU, 0x47bf94e9U,
        0x8cbcce22U, 0x559fc06aU, 0xd268f536U, 0xe10af79aU,
        0xc1af4d69U, 0x1d2917b5U, 0xec4c304dU, 0x9ee5016cU,
        0x69232f74U, 0xfead7bb3U, 0xe9089ab6U, 0xf012f6aeU,
    };

    if (!key) return 0;

    const char *str = (const char *) key;
    hash_val_t acc = 0;

    while (*str) {
        acc ^= randbox[(tolower(*str) + acc) & 0xf];
        acc = (acc << 1) | (acc >> 31);
        acc &= 0xffffffffU;
        acc ^= randbox[((tolower(*str++) >> 4) + acc) & 0xf];
        acc = (acc << 2) | (acc >> 30);
        acc &= 0xffffffffU;
    }
    return acc;
}

int EqualCaseCharPtr (const void *x, const void *y) {
        if (!x && !y) return 1;
        if (!x || !y) return 0;
        return stricmp((const char *)x, (const char *)y);
};

