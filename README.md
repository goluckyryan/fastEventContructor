# Intoduction

This **EventBuilder** use the **BinaryReader** class that read raw binary file. the data can be saved to **Hit** class. 

The **Hit** class only store the GEBHeader and payload. It can decode the payload into **Event** class. 

There are many information in the payload, but only the **id** (board_id * 10 + channel_id), **pre_rise_energy**, **post_rise_energy**, **timestamp** are important.

The **EventBuilder** can also save the trace and it outputs CERN ROOT TTree. 


## EventBuilder

This is the original EventBuilder. The files are grouped by the DigiID and scanned. it grapes a batch of data in the frist file of each digiID. It has no intermediate file output but directly save the runXXX.root. The scanning of files is running in parallel to speed thing up. By using piority_queue, each file push data to the queue and the queue always give the smallest timesatmp data. That reduced the looping of all file to find the earleist time.

# Experimental Parquet format (not implemented)

parquet format is also coulomb wise 

## install the parquest package

https://arrow.apache.org/install/

```sh
sudo apt install -y -V libarrow-dev libparquet-dev
```