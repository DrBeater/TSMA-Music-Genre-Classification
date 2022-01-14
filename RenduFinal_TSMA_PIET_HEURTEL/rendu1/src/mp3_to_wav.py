import os
import glob

# files                                                                         
lst = glob.glob("*.mp3")
print(lst)
 
for file in lst:
# convert wav to mp3
	os.system(f"""ffmpeg -i {file} -acodec pcm_u8 -ar 22050 {file[:-4]}.wav""")  