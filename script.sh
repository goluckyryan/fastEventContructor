#file name must be either expName_XXX_YYY_ZZZ_Q
#                or  dgs_run87.gtd11_000_0140_9

#./EventBuilder test.root 1000  `\ls -1 data/dgs_run116/*.gtd* | grep -v _F`
#./EventBuilder test.root 1000  `\ls -1 data_slopebox/kaka_002*`

#./EventBuilder test.root 1000  `\ls -1 data/haha_002*`

# ./EventBuilder_S tac2_021.root 1000 1 20 `\ls -1 data/TAC2_021/* | grep -v _T`

# ./EventBuilder_S tac2_204_single.root -1 1 30 `\ls -1 data/TAC2_204/* | grep -v _T`
# ./EventBuilder_S tac2_204_single.root -1 1 30 `\ls -1 data/TAC2_204/*`


# ./EventBuilder_S testD_005_trace.root 100 1 1 30 `\ls -1 data_slopebox/testD_005/*`
./EventBuilder_S testD_004_trace.root 100 1 1 30 `\ls -1 data_slopebox/testD_005/*`

# ./EventBuilder testD_003.root 100 1 `\ls -1 data_slopebox/testD_003/*`
# ./EventBuilder testD_004.root 100 1 `\ls -1 data_slopebox/testD_004/*`
# ./EventBuilder testD_005.root 100 1 `\ls -1 data_slopebox/testD_005/*`

#exit

# clear
# root 'analyzer_trace.cpp("tac2_021.root")'

# root 'analyzer_tac.cpp("testD_004.root")'
# root 'analyzer_tac.cpp("root_data/TAC2_044_1.root")'
# root 'analyzer_tac.cpp("root_data/TAC2_022_0.root")'

# root 'analyzer.cpp("testD_005.root")'
# root 'analyzer.cpp("root_data/TAC2_022_1.root")'

# root 'readTrace_S.C("tac2_021_single.root", 12107, 12107)'

# root 'analyzer_trace.cpp("tac2_021_single.root")'

# root analyzer_script.cpp