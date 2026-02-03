# R r a h t A
音程 节奏 响度 和声 音色 标签 
___
## R
### Related Pitch
### 方向：关于识别音与音之间的距离的能力
#### 科目：按照上行下行的两种顺序播放对象内容序列
##### 实现：
1. 输入： 标签名称
2. 查找： 包含的音符名称数组
3. 限制： 不包含强制的八度标签
4. 加工： 
5. 播放： 频上行，频率下行




___
## r
### rhythm
### 方向：关于识别节奏
#### 科目：按照上行下行的两种顺序播放 interval chord
##### 实现：

___
## a
### amplitude
### 方向：关于识别
#### 科目：按照不同响度播放 interval chord
##### 实现：

___
## h
### harmonic
### 方向：关于音符共同播放时的识别能力
#### 科目：同时播放
##### 实现：

___
## t
### 
### 方向：关于音符共同播放时的识别能力
#### 科目：同时播放
##### 实现：



___

double hard;
// double R, r, a, h, t, A;
double a, b, c, d, e, f, g ... n;
// ESkillClass eSkill = initialEarSkill(R, r, a, h, t, A);
ESkillClass eSkill = initialEarSkill(a, b ... n);

basicDifficulty -> d0 -> h1 -> ivl ->
std::string target[num_learn] = selectTodayCards(ivl);
->  cardClass c = selectCard(target); ->  metadataArray
->  c.a, c.b, c.c, c.d, c.e, c.f ... c.n [originalMetadataArray]
->  combination() -> a1, a2, a3 ... ai ... an [combinationArray]
    // Modelling algoEntity by eSkill
->  AlgoEntityClass algoEntity = initialCostMapper(eSkill);
->  i.cost = algoEntity.audioCostMapper(ai.audio) -> ordered by cost
->  b1, b2, b3 ... bi ... bn [costArray]
->  
    auto combinationPointer;
    for(auto b : costArray) 
        if(b < hard) {
            combinationPointer = b;
            std::string outcome_str =  recall(b);
        }
    bool outcome_data = outcomeProcessor(outcome_str, trust);
    DataClass data = f(outcome_data, recall, d0, h0);
    d1 = data.d1; h1 = data.h1;
    updateTarget(d1, h1);
    // Updating algoEntity by learning data
    algoEntity.updateMetadata(
        combinationPointer, 
        outcome_data, 
        d1, h1);
    // Ideally.
    if(combinationPointer.contain(i) && outcome_data == false)
        original.i > current.i;
    /** 
     * The algo model will use different stratage to recall
     * the same target's metadata because we update the 
     * eSkill, and the model initialed by eSkill.
     */


opus arc 
260203

___

// double R, r, a, h, t, A;
double a, b, c, d, e, f, g ... n;
// ESkillClass eSkill = initialEarSkill(R, r, a, h, t, A);


h1 = f(eSkill);
d0 = _f(h1);
ivl = f(h1, d0);

ESkillClass eSkill = initialEarSkill(a, b ... n);

double epsilon = ?;
while(){
    audio = randomAudio(d0);
    _d = randomForest(audio);
    if(abs(_d - d0) < epsilon){
        targetAudio = audio;
        targetD = _d;
        break;
    }
}
h2 = f(outcome_data, targetD, h1);
attributionArray = SHAP(targetAudio);
eSkill.reset(updateFunc(attributionArray, h1, h2));


Fancy
260203

___




























