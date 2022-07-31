gcc example_1_sender.c -o sender
#each time we run this another message is sent
sudo docker build --no-cache -t sender .
sudo dockerd & > /dev/null 2>&1