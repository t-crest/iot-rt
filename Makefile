# Runs server and a client
all:
	sbt "runMain iotrt.BlaaHund dbaa8026.ngrok.io"

server:
	sbt "runMain iotrt.App"

client:
	sbt "runMain iotrt.BlaaHundClient dbaa8026.ngrok.io"


app:
	sbt "runMain iotrt.App"
