#!/usr/bin/env python3
import argparse
import os

HEADER_TEMPLATE = """// Auto-generated monolog config
#pragma once

#include <string_view>
#include <vector>

namespace Lang {{
    namespace Monolog {{
        struct SoundInfo {{
            const char* filename;
            std::string_view data;
        }};

{sounds}
        static const std::vector<SoundInfo> SOUNDS = {{
{sounds_array}
        }};
    }}
}}
"""

def get_sound_files(directory):
    if not os.path.exists(directory):
        return []
    return [f for f in os.listdir(directory) if f.endswith('.ogg')]

def generate_header(input_dir, output_path):
    sounds_list = get_sound_files(input_dir)
    
    sounds = []
    sounds_array = []
    
    for file in sorted(sounds_list):
        base_name = os.path.splitext(file)[0]
        # In esp-idf, the symbol is usually _binary_filename_ext_start
        # So for dummy.ogg -> _binary_dummy_ogg_start
        sounds.append(f'''        extern const char ogg_{base_name}_start[] asm("_binary_{base_name}_ogg_start");
        extern const char ogg_{base_name}_end[] asm("_binary_{base_name}_ogg_end");''')
        
        sounds_array.append(f'''            {{
                "{file}",
                std::string_view(
                    static_cast<const char*>(ogg_{base_name}_start),
                    static_cast<size_t>(ogg_{base_name}_end - ogg_{base_name}_start)
                )
            }}''')

    # Provide a dummy array if empty
    if not sounds_list:
        content = """// Auto-generated monolog config (EMPTY)
#pragma once
#include <string_view>
#include <vector>
namespace Lang {
    namespace Monolog {
        struct SoundInfo {
            const char* filename;
            std::string_view data;
        };
        static const std::vector<SoundInfo> SOUNDS;
    }
}
"""
    else:
        content = HEADER_TEMPLATE.format(
            sounds="\n".join(sounds),
            sounds_array=",\n".join(sounds_array)
        )

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(content)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate monolog configuration header")
    parser.add_argument("--input_dir", required=True, help="Directory containing .ogg files")
    parser.add_argument("--output", required=True, help="Output header file path")
    args = parser.parse_args()

    try:
        generate_header(args.input_dir, args.output)
        print(f"Successfully generated monolog config file: {args.output}")
    except Exception as e:
        print(f"Error: {e}")
        exit(1)
