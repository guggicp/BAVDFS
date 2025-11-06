import requests
import hashlib

def get_file_md5(url):
    response = requests.get(url)
    md5_hash = hashlib.md5()
    md5_hash.update(response.content)
    return md5_hash.hexdigest()

file_url = "https://github.com/microsoft/DiskANN/archive/refs/tags/0.7.0.tar.gz"
file_md5 = get_file_md5(file_url)
print("File MD5: ", file_md5)