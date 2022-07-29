gcc example_1_receiver.c -o receiver
docker build -t receiver .
#Run this on differnt termials to have how many receivers you would like
docker run -it receiver /app/receiver 