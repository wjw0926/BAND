ffmpeg -i sample1.avi -i sample2.avi -filter_complex "[0:a][1:a]amerge=inputs=2[aout]" -map "[aout]" -ac 2 output.mp3
ffmpeg -i output.mp3 -i result.avi final.avi