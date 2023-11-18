import sys
import subprocess
import os
import dotenv
import explore
import interface
dotenv.load_dotenv()

def update_reqs():
    reqs_flag = os.getenv("INSTALLED")
    print(reqs_flag)
    if reqs_flag:
        pass
    else:
        subprocess.check_call([
                    sys.executable,
                    "-m",
                    "pip",
                    "install",
                    "-r",
                    "requirements.txt"
                ])
        dotenv.set_key(".env", key_to_set="INSTALLED", value_to_set="True")


if __name__ == '__main__':
    update_reqs()
    interface.main()
