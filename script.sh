#file name must be either expName_XXX_YYY_ZZZ_Q
#                or  dgs_run87.gtd11_000_0140_9

#./EventBuilder test.root 1000  `\ls -1 data/dgs_run116/*.gtd* | grep -v _F`
#./EventBuilder test.root 1000  `\ls -1 data_slopebox/kaka_002*`

#./EventBuilder test.root 1000  `\ls -1 data/haha_002*`

# ./EventBuilder_S tac2_021.root 1000 1 20 `\ls -1 data/TAC2_021/* | grep -v _T`

./EventBuilder_S tac2_204_single.root -1 1 30 `\ls -1 data/TAC2_204/* | grep -v _T`
# ./EventBuilder_S tac2_204_single.root -1 1 30 `\ls -1 data/TAC2_204/*`

exit

clear
# root 'analyzer_trace.cpp("tac2_021.root")'

# root 'analyzer_tac.cpp("root_data/TAC2_022_1.root")'
#root 'analyzer_tac.cpp("root_data/TAC2_044_1.root")'
# root 'analyzer_tac.cpp("root_data/TAC2_022_0.root")'


# root 'analyzer.cpp("root_data/TAC2_022_1.root")'

# root 'readTrace_S.C("tac2_021_single.root", 12107, 12107)'

# root 'analyzer_trace.cpp("tac2_021_single.root")'

root analyzer_script.cpp