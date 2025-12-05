import os

# Handle case sensitivity or user preference
folder = "Samples/Lofi Piano"
if not os.path.exists(folder):
    folder = "samples/Lofi Piano"

print(f"Processing folder: {folder}")

files = [f for f in os.listdir(folder) if f.endswith(".wav")]

# Mapping logic based on user description:
# "3" (sounds high) -> 5
# "4" (sounds low) -> 3
# "5" (sounds mid) -> 4

# We use a temp prefix to avoid collisions during renaming
renames = []

for f in files:
    # Check octave in filename
    # Assuming format like "C#3.wav" or "C3.wav"
    # We look for the digit just before .wav
    name_part = f.replace(".wav", "")
    if not name_part[-1].isdigit(): continue
    
    octave = int(name_part[-1])
    note_part = name_part[:-1]
    
    new_octave = -1
    
    if octave == 3:
        new_octave = 5
    elif octave == 4:
        new_octave = 3
    elif octave == 5:
        new_octave = 4
    
    if new_octave != -1:
        new_name = f"{note_part}{new_octave}.wav"
        # Use a temp name first
        temp_name = f"__TEMP__{new_name}"
        renames.append((f, temp_name, new_name))

# Pass 1: Rename to temp
for old, temp, new in renames:
    old_path = os.path.join(folder, old)
    temp_path = os.path.join(folder, temp)
    print(f"Stage 1: {old} -> {temp}")
    os.rename(old_path, temp_path)

# Pass 2: Rename temp to final
for old, temp, new in renames:
    temp_path = os.path.join(folder, temp)
    final_path = os.path.join(folder, new)
    print(f"Stage 2: {temp} -> {new}")
    os.rename(temp_path, final_path)

print("Swap complete.")
