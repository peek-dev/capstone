python3 gen_bins.py calibration_data_temp.txt | sed -E 's/([0-9]) /\1, /g;s/\]/],/g;s/\],\],\],/]]]/g
