# Install

```
pip install -r requirements.txt
```

# Build

```
protoc --python_out=. data.proto
```

# Run Webserver

```
python main.py
```

`CTRL-C` to quit.

## Usage

Takes `stdin` input and populates global `protobuf` with parsed information. For example, for a protobuf with the following structure

```
message Data {
    int32 id = 1;
    int32 value = 2;
    float timestamp = 5;
    string opts = 6;
}
```

(`proto` must have integer `id` field)

input `--value 123 --timestamp 123.4 --opts hello` to yield a `protobuf` containing the information through an http request. Input `--help` for more information (uses `argparse` for parsing). All input parameters are optional, and errors are ignored: `--value 123` will update only `Data.value <- 123`.

## Testing

`test_app.py` contains a simple loop requesting from `LOCALHOST` and deserializing the received protobuf. Run via 

```
python test_app.py
```

This is designed to run concurrently with `main.py`. `CTRL-C` to quit.
