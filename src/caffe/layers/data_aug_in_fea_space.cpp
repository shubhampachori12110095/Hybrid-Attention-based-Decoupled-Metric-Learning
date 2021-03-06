//随机产生Wk [0-1]且和为1 精度为precision 
/*
#include <vector>
#include <fstream>  // NOLINT(readability/streams)
#include <string>
#include <iostream>  // NOLINT(readability/streams)
#include <time.h>

#include "caffe/filler.hpp"
#include "caffe/layers/data_aug_in_fea_space.hpp"
#include "caffe/util/math_functions.hpp"

namespace caffe {

template <typename Dtype>
void DataAuginFeaSpaceLayer<Dtype>::LayerSetUp(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top) {
  CHECK_EQ(bottom[0]->num(), bottom[1]->num())<< "Input size does not match";
}

template <typename Dtype>
void DataAuginFeaSpaceLayer<Dtype>::Reshape(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top) {
  CHECK_EQ(bottom[0]->num(), bottom[1]->num())
      << "Input size does not match.";
  vector<int> top_shape = bottom[0]->shape();
  top[0]->Reshape(top_shape);
  //计算同类的个数
  int sum = 0;
  for(int i = 0; i < bottom[0]->num(); i++)
  {
        if(bottom[1]->cpu_data()[i] == bottom[1]->cpu_data()[0])
                sum++;
  }
  w_.Reshape(1,1,sum,sum);
}

template <typename Dtype>
void DataAuginFeaSpaceLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
    const vector<Blob<Dtype>*>& top) {
    const Dtype* data = bottom[0]->cpu_data();
    const Dtype* label = bottom[1]->cpu_data();
    Dtype* top_data = top[0]->mutable_cpu_data();
    
    const int num = bottom[0]->num();
    const int channels = bottom[0]->channels();
    Dtype precision = this->layer_param_.data_aug_in_fea_space_param().precision();
    //generate random W_
    for(int j = 0; j < w_.width(); j++)
    {
        Dtype sum = Dtype(0);
        for(int i = 0; i < w_.height(); i++)
        {
                w_.mutable_cpu_data()[j * w_.height() + i] = rand()%(static_cast<int>(Dtype(1)/precision));
                sum+=w_.mutable_cpu_data()[j * w_.height() + i];
                
        }
        for(int i = 0; i < w_.height(); i++)
        {
                w_.mutable_cpu_data()[j * w_.height() + i] = w_.mutable_cpu_data()[j * w_.height() + i] / sum;
                //std::cout<<w_.mutable_cpu_data()[j * w_.height() + i]<<"  ";
        }
        //std::cout<<std::endl;
    }
    //generate data
    caffe_set(top[0]->count(), Dtype(0), top[0]->mutable_cpu_data());
    int w_index = 0;
    for(int i = 0; i < num; i++)
    {
        //find the same class indexes.
        std::vector<int> index;
        for(int j = 0; j < num; j++)
        {
                if(index.size()==w_.width())break;
                if(static_cast<int>(label[i])==static_cast<int>(label[j]))
                        index.push_back(j);
        }
        //generate new data_i;
        for(int j = 0; j < w_.width(); j++)
        {
                caffe_cpu_axpby(channels, w_.cpu_data()[w_index * w_.width() + j], data + index[j] * channels, Dtype(1), top_data + i * channels);
        }
        w_index++;
        w_index = (w_index)%w_.width();
    }
    
}

template <typename Dtype>
void DataAuginFeaSpaceLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top,
    const vector<bool>& propagate_down,
    const vector<Blob<Dtype>*>& bottom) {
    //compute bottom gradients
    if (propagate_down[0]) {
        const int num = bottom[0]->num();
        const int channels = bottom[0]->channels();
        const Dtype* label = bottom[1]->cpu_data();
        
        caffe_set(bottom[0]->count(), Dtype(0), bottom[0]->mutable_cpu_diff());
        int w_index = 0;
        for(int i = 0; i < num; i++)
        {
                //find the same class indexes.
                std::vector<int> index;
                for(int j = 0; j < num; j++)
                {
                        if(index.size()==w_.width())break;
                        if(static_cast<int>(label[i])==static_cast<int>(label[j]))
                                index.push_back(j);
                }
                //compute gradients from data_i;
                for(int j = 0; j < w_.width(); j++)
                {
                        caffe_cpu_axpby(channels, w_.cpu_data()[w_index * w_.width() + j], top[0]->cpu_diff() + i * channels, Dtype(1), bottom[0]->mutable_cpu_diff() + index[j] * channels);
                        
                }
                w_index++;
                w_index = (w_index)%w_.width();
        }
    }
}

#ifdef CPU_ONLY
STUB_GPU(DataAuginFeaSpaceLayer);
#endif

INSTANTIATE_CLASS(DataAuginFeaSpaceLayer);
REGISTER_LAYER_CLASS(DataAuginFeaSpace);

} 
*/
//随机产生外插数据 外插幅度为precision
#include <vector>
#include <fstream>  // NOLINT(readability/streams)
#include <string>
#include <iostream>  // NOLINT(readability/streams)
#include <time.h>

#include "caffe/filler.hpp"
#include "caffe/layers/data_aug_in_fea_space.hpp"
#include "caffe/util/math_functions.hpp"

namespace caffe {

template <typename Dtype>
void DataAuginFeaSpaceLayer<Dtype>::LayerSetUp(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top) {
  CHECK_EQ(bottom[0]->num(), bottom[1]->num())<< "Input size does not match";
}

template <typename Dtype>
void DataAuginFeaSpaceLayer<Dtype>::Reshape(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top) {
  CHECK_EQ(bottom[0]->num(), bottom[1]->num())
      << "Input size does not match.";
  vector<int> top_shape = bottom[0]->shape();
  top[0]->Reshape(top_shape);
  w_.Reshape(1,1,bottom[0]->num(),2);
}

template <typename Dtype>
void DataAuginFeaSpaceLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
    const vector<Blob<Dtype>*>& top) {
    const Dtype* data = bottom[0]->cpu_data();
    const Dtype* label = bottom[1]->cpu_data();
    Dtype* top_data = top[0]->mutable_cpu_data();
    
    const int num = bottom[0]->num();
    const int channels = bottom[0]->channels();
    Dtype precision = this->layer_param_.data_aug_in_fea_space_param().precision();//control the range of extrapolation
    
    //generate data i
    caffe_set(top[0]->count(), Dtype(0), top[0]->mutable_cpu_data());
    for(int i = 0; i < num; i++)
    {
        vector<int> index;
        for(int j = 0; j < num; j++)
        {
                if(static_cast<int>(label[i])==static_cast<int>(label[j]))
                        index.push_back(j);
        }
        //choose random pair for generating new data i
        std::random_shuffle(index.begin(),index.end());
        w_.mutable_cpu_data()[i*2] = Dtype(index[0]);
        w_.mutable_cpu_data()[i*2+1] = Dtype(index[1]);
        //std::cout<<index[0]<<" "<<index[1]<<std::endl;
        
        caffe_cpu_axpby(channels, Dtype(1+precision), data + static_cast<int>(w_.cpu_data()[i*2]) * channels, Dtype(1), top_data + i * channels);
        caffe_cpu_axpby(channels, Dtype(-1)*precision, data + static_cast<int>(w_.cpu_data()[i*2+1]) * channels, Dtype(1), top_data + i * channels);
    }
    
}

template <typename Dtype>
void DataAuginFeaSpaceLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top,
    const vector<bool>& propagate_down,
    const vector<Blob<Dtype>*>& bottom) {
    //compute bottom gradients
    if (propagate_down[0]) {
        const int num = bottom[0]->num();
        const int channels = bottom[0]->channels();
        Dtype precision = this->layer_param_.data_aug_in_fea_space_param().precision();
        
        caffe_set(bottom[0]->count(), Dtype(0), bottom[0]->mutable_cpu_diff());
        for(int i = 0; i < num; i++)
        {
               //compute gradients from generated data_i
               caffe_cpu_axpby(channels, Dtype(1+ precision), top[0]->cpu_diff() + i * channels, Dtype(1), bottom[0]->mutable_cpu_diff() + static_cast<int>(w_.cpu_data()[i*2])*channels);
               caffe_cpu_axpby(channels, Dtype(-1)*precision, top[0]->cpu_diff() + i * channels, Dtype(1), bottom[0]->mutable_cpu_diff() + static_cast<int>(w_.cpu_data()[i*2+1])*channels);
        }
    }
}

#ifdef CPU_ONLY
STUB_GPU(DataAuginFeaSpaceLayer);
#endif

INSTANTIATE_CLASS(DataAuginFeaSpaceLayer);
REGISTER_LAYER_CLASS(DataAuginFeaSpace);

} 
