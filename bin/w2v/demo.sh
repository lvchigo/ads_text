mpwd=`pwd`
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$mpwd/../../lib

keyfile='/home/chigo/working/word2vec/word2vec/keyfile'
#keyfile='/home/xiaogao/job/word2vec/keyfile'

#get ads string
#../Demo_w2v -getadsstring ads_photodesc_ubuntu.csv UrlResult.txt AdsResult.txt

#get noads string
#binLabel=1,pos;binLabel=0,neg;
#../Demo_w2v -getnoadsstring Train/add_pos.txt Train/add_pos_sv.txt 1
#../Demo_w2v -getnoadsstring Train/add_neg.txt Train/add_neg_sv.txt 0

#clearsourcedata
#binString=1,"id string label";binString=0,"string";
#../Demo_w2v -clearsourcedata /home/chigo/working/word2vec/data/in_data/jianzhi_shuying_ubuntu.csv /home/chigo/working/word2vec/data/in_data/jianzhi_shuying_rmsim.txt 0

#rmsimstring
#binString=1,"id string label";binString=0,"string";
#../Demo_w2v -rmsimstring /home/chigo/working/word2vec/data/in_data/desc.csv /home/chigo/working/word2vec/data/in_data/desc_rmsim.csv 0
#../Demo_w2v -rmsimstring Train/Train_Ads_NoAds_150w.txt Train/Train_Ads_NoAds_150w_rmsim.txt 1
#../Demo_w2v -rmsimstring Data/in_ads_comment.csv Data/in_ads_comment_rmsim.txt 0

#cutword	binFeat:1-id string label;0-string;2-id string;
#../Demo_w2v -cutword Data/Alldata_rmsim.txt Cutword/Alldata_rmsim_cutword.txt 0
#../Demo_w2v -cutword Data/in_sogou.txt Cutword/in_sogou_cutword.txt 0

#countidf
#../Demo_w2v -countidf Cutword/Alldata_rmsim_cutword.txt Cutword/Alldata_rmsim_idf.txt
#../Demo_w2v -countidf Cutword/in_sogou_cutword.txt Cutword/in_sogou_idf.txt

#Train word2vec
#../Demo_word2vec -train Cutword/Alldata_rmsim_cutword.txt -output Cutword/Alldata_rmsim.vec -cbow 0 -size 200 -window 10 -negative 0 -hs 1 -sample 1e-3 -threads 4 -binary 1
#../Demo_word2vec -train Cutword/in_sogou_cutword.txt -output Cutword/in_sogou.vec -cbow 0 -size 200 -window 10 -negative 0 -hs 1 -sample 1e-3 -threads 4 -binary 1

#pca Feat
#../Demo_w2v -pca PCA/Train_Ads_NoAds.txt $keyfile PCA/pca_text2class_150902.model PCA/outPCAFeat

#train binFeat:1-feat by pca;0-feat by doc2vec
#../Demo_w2v -train PCA/Train_Ads_NoAds.txt $keyfile Train/Train_Ads_NoAds_Feat 0
#../Demo_w2v -train Train/train_100w_150902/Train_Ads_NoAds_100w.txt $keyfile Train/train_100w_150902/Train_Ads_NoAds_100w_Feat 1 
#../Demo_w2v -train Train/train_seed/train_2015.10.16/train_pos_neg.txt $keyfile Train/train_seed/train_2015.10.16/train_pos_neg_Feat 1 

#predict binFeat:1-id string label;0-string;2-id string;
#../Demo_w2v -predict Train/train_seed/jianzhi_shuying_rmsim.txt $keyfile Test/jianzhi_all.txt Test/jianzhi_all_1.txt 0
#../Demo_w2v -predict Test/ads_online_test150906.csv $keyfile Test/ads_online_test150906_res.txt Test/ads_online_test150906_res_1.txt 0
#../Demo_w2v -predict Test/predict_2015.10.7/neg_res.txt $keyfile Test/predict_2015.10.7/neg_res_1.txt Test/predict_2015.10.7/neg_res_2.txt 2
#../Demo_w2v -predict_shorttext Test/ads_online_test150906_test.csv $keyfile Test/ads_online_test150906_res.txt Test/ads_online_test150906_res_1.txt 0
#../Demo_w2v -predict_shorttext Data/in_ads_imgtext_comment.txt $keyfile in_res.txt in_res1.txt 0
#../Demo_w2v -predict_shorttext Test/ads_online_test150906.csv $keyfile in_res.txt in_res1.txt 0
../Demo_w2v -predict_2 Test/ads_online_test150906.csv $keyfile Test/ads_online_test150906_res.txt Test/ads_online_test150906_res_1.txt 0
#../Demo_w2v -predict_2 Test/predict_2015.10.7/neg_res.txt $keyfile Test/neg_res_1.txt Test/neg_res_2.txt 2
#../Demo_w2v -predict_2 test.txt $keyfile test_1.txt test_2.txt 0

#getsample binFeat:1-id string label;0-string
#../Demo_w2v -getsample Train/train_seed/Train_Ads_NoAds_13000.txt $keyfile Train/train_seed/add_pos.txt Train/train_seed/add_neg.txt 1
#../Demo_w2v -getsample Train/train_100w_150902/Train_Ads_NoAds_100w.txt $keyfile Train/train_100w_150902/add_pos.txt Train/train_100w_150902/add_neg.txt 1

#gettrainsample binFeat:1-id string label;0-string
#../Demo_w2v -gettrainsample Train/train_seed/train_2015.10.16/seed_2015.10.20.txt $keyfile Train/train_seed/train_2015.10.16/seed_1.txt Train/train_seed/train_2015.10.16/seed_2.txt 1
#../Demo_w2v -gettrainsample Data/in_ads_imgtext_comment.txt $keyfile in_1.txt in_2.txt 0

#rmtrainsample
#../Demo_w2v -rmtrainsample Train/train_seed/train_2015.10.16/good/seed_1.txt $keyfile Train/train_seed/train_2015.10.16/good/seed_1_rm.txt 1
#../Demo_w2v -rmtrainsample Train/train_seed/train_2015.10.16/train_pos_neg.txt $keyfile Train/train_seed/train_2015.10.16/train_pos_neg_rm.txt 1

#addsample binFeat:1-id string label;0-string
#../Demo_w2v -addsample Data/Alldata_rmsim_addsample_head400w.txt $keyfile add_pos.txt add_neg.txt 0

#findsimword binFeat:1-id string label;0-string
#../Demo_w2v -findsimword Test/ads_simword/adsword_shuying_ubuntu.csv $keyfile Test/ads_simword/ads_simword_res.txt Test/ads_simword/ads_simword1.txt 0
#../Demo_w2v -findsimword zhongkouwei_word.txt $keyfile zhongkouwei_simword.txt 0

#filterstring
#../Demo_w2v -filterstring Test/ads_online_test150906.csv filterstring.txt
#../Demo_w2v -filterstring Test/ads_online_test150906_test.csv filterstring.txt

#test
#../Demo_w2v -test

#kdtree
#../Demo_kdtree
