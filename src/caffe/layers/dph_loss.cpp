//binomial deviance loss for hashing
#include <algorithm>
#include <vector>
#include "stdio.h"
#include "caffe/filler.hpp"
#include "caffe/layer.hpp"
#include "caffe/layers/dph_loss.hpp"
#include "caffe/util/io.hpp"
#include "caffe/util/math_functions.hpp"
#include <cmath>
/**********************/
/**/
namespace caffe {

template <typename Dtype>
void DPHLossLayer<Dtype>::LayerSetUp(
  const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
  LossLayer<Dtype>::LayerSetUp(bottom, top);
  CHECK_EQ(bottom[0]->height(), 1);
  CHECK_EQ(bottom[0]->width(), 1);
  center_temp_.Reshape(1,bottom[0]->channels(), 1, 1); //存储临时差量
  diff_.Reshape(bottom[0]->num(),bottom[0]->channels(), 1, 1);
  one_.Reshape(1,bottom[0]->channels(),1,1);
}
template <typename Dtype>
void DPHLossLayer<Dtype>::Forward_cpu(
    const vector<Blob<Dtype>*>& bottom,
    const vector<Blob<Dtype>*>& top) {
    const Dtype* data = bottom[0]->cpu_data();
    const Dtype* label = bottom[1]->cpu_data();
    Dtype* one = one_.mutable_cpu_data();
    int num = bottom[0]->num();//
    int channels = bottom[0]->channels();
    Dtype beta = this->layer_param_.dph_loss_param().beta();
    Dtype alpha = this->layer_param_.dph_loss_param().alpha();
    Dtype cost_p = this->layer_param_.dph_loss_param().cost_p();
    Dtype cost_n = this->layer_param_.dph_loss_param().cost_n();
    Dtype gamma = this->layer_param_.dph_loss_param().gamma();//for value constraint
    Dtype gamma1 = this->layer_param_.dph_loss_param().gamma1();//for binomial loss
    caffe_set(channels, Dtype(1.0), one);
    
    Dtype loss = 0;
    Dtype* diff = diff_.mutable_cpu_data();
    Dtype* center_temp = center_temp_.mutable_cpu_data();
    caffe_set(channels*num, Dtype(0), diff);
    top[0]->mutable_cpu_data()[0]=0;
    //compute wij for positive number and negative number
    int w_pos=0, w_neg=0;
    for(int i=0; i<num; i++)
        for(int j=i+1; j<num; j++)
            label[i]==label[j] ? w_pos++ : w_neg++;
    
    for(int i=0; i<num; i++)
    {
        Dtype norm_i = std::sqrt(caffe_cpu_dot(channels, data + i * channels, data + i * channels));
        for(int j=i+1; j<num; j++)
	{
                                int w = label[i]==label[j] ? w_pos : w_neg;
                                Dtype inner = caffe_cpu_dot(channels, data + i * channels, data + j * channels);
                                Dtype norm_j = std::sqrt(caffe_cpu_dot(channels, data + j * channels, data + j * channels));
                                Dtype param = label[i]==label[j] ? (-1*alpha*cost_p) : (alpha*cost_n);
                                Dtype ans = exp(param*(inner/(norm_i*norm_j)-beta));
                                loss = log(1+ans); 
		                top[0]->mutable_cpu_data()[0] = top[0]->mutable_cpu_data()[0] + gamma1*loss/w;
		                
		                //gradients
		                //xi
		                caffe_copy(channels, data + j * channels, center_temp);
		                caffe_cpu_axpby(channels, Dtype(-1*inner/(norm_i*norm_i*norm_i*norm_j)), data + i * channels, Dtype(1/(norm_i*norm_j)), center_temp);
		                caffe_cpu_axpby(channels, Dtype(param*ans/(w+w*ans)), center_temp, Dtype(1), diff + i * channels);
		                //xj
		                caffe_copy(channels, data + i * channels, center_temp);
		                caffe_cpu_axpby(channels, Dtype(-1*inner/(norm_j*norm_j*norm_j*norm_i)), data + j * channels, Dtype(1/(norm_i*norm_j)), center_temp);
		                caffe_cpu_axpby(channels, Dtype(param*ans/(w+w*ans)), center_temp, Dtype(1), diff + j * channels);
	
	}
	caffe_abs<Dtype>(channels, data + i * channels, center_temp);
	caffe_cpu_axpby(channels, Dtype(-1), one, Dtype(1), center_temp);
	caffe_abs<Dtype>(channels, center_temp, center_temp);
	loss = caffe_cpu_asum(channels, center_temp);
	top[0]->mutable_cpu_data()[0] = top[0]->mutable_cpu_data()[0] + loss*gamma/num;
	
	//caffe_abs<Dtype>(channels, data + i * channels, center_temp);
	//LOG(INFO)<<caffe_cpu_asum(channels,center_temp)/channels;
    }
}

     

template <typename Dtype>
void DPHLossLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top,
	const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) 
{
    int num = bottom[0]->num();//
    int channels = bottom[0]->channels(); 
    Dtype gamma = this->layer_param_.dph_loss_param().gamma();
    Dtype gamma1 = this->layer_param_.dph_loss_param().gamma1();
    //update gradients
    caffe_cpu_axpby(bottom[0]->count(), top[0]->cpu_diff()[0] * gamma1, diff_.cpu_data(), Dtype(0.0), bottom[0]->mutable_cpu_diff());
    for(int i = 0; i < num * channels; i++)
    {
        if(bottom[0]->cpu_data()[i]>=1.0 || (bottom[0]->cpu_data()[i]<=0.0 && bottom[0]->cpu_data()[i]>=-1.0))
            bottom[0]->mutable_cpu_diff()[i] += gamma/num;
        else
            bottom[0]->mutable_cpu_diff()[i] -= gamma/num;
    }
}




//#ifdef CPU_ONLY
//STUB_GPU(DPHLossLayer);
//#endif

INSTANTIATE_CLASS(DPHLossLayer);
REGISTER_LAYER_CLASS(DPHLoss);

}  // namespace caffe
