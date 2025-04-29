#!/usr/bin/env python
# coding: utf-8

# # GA-ExtractiveOracle generate labelled data

# ## Start of [1]: Import data

# In[1]:


import pandas as pd


# Show training data head

# In[2]:


pd.set_option("display.max_columns", None)
train = pd.read_csv("Dataset/train.csv")
val = pd.read_csv("Dataset/dev.csv")


# ## End of [1]: Import data

# ## Start of [2]: Span recognition (sentence splitting in this model)

# In[6]:


import nltk
nltk.download('punkt')
nltk.download('punkt_tab')


# In[7]:


from nltk.tokenize import sent_tokenize, word_tokenize

# In[8]:


train_chapters = train['chapter'].tolist()
train_chapters_paragraphs = [train_chapters[i].split("\n\n") for i in range(len(train_chapters))]
for i in range(len(train_chapters_paragraphs)):
    train_chapters_paragraphs[i] = [train_chapters_paragraphs[i][j].replace("\n", " ") for j in range(len(train_chapters_paragraphs[i]))]
train_chapters = [train_chapters[i].replace("\n", " ") for i in range(len(train_chapters))]
val_chapters = val['chapter'].tolist()
val_chapters_paragraphs = [val_chapters[i].split("\n\n") for i in range(len(val_chapters))]
for i in range(len(val_chapters_paragraphs)):
    val_chapters_paragraphs[i] = [val_chapters_paragraphs[i][j].replace("\n", " ") for j in range(len(val_chapters_paragraphs[i]))]
val_chapters = [val_chapters[i].replace("\n", " ") for i in range(len(val_chapters))]

# In[9]:


train_chapters_sentences = [sent_tokenize(train_chapters[i]) for i in range(len(train_chapters))]
val_chapters_sentences = [sent_tokenize(val_chapters[i]) for i in range(len(val_chapters))]

train_chapters_paragraphs_sentences = []
for i in range(len(train_chapters_paragraphs)): # chapter i
    chapter_list = []
    for j in range(len(train_chapters_paragraphs[i])): # paragraph j
        chapter_list.append(sent_tokenize(train_chapters_paragraphs[i][j]))
    train_chapters_paragraphs_sentences.append(chapter_list)
val_chapters_paragraphs_sentences = []
for i in range(len(val_chapters_paragraphs)): # chapter i
    chapter_list = []
    for j in range(len(val_chapters_paragraphs[i])): # paragraph j
        chapter_list.append(sent_tokenize(val_chapters_paragraphs[i][j]))
    val_chapters_paragraphs_sentences.append(chapter_list)


# ## End of [2]: Span recognition

# ## Start of [3]: Training

# In[10]:


# Calculate average summary length vs chapter length ratio
train_chapters_length = train['chapter_length'].tolist()
train_summaries_length = train['summary_length'].tolist()
val_chapters_length = val['chapter_length'].tolist()
average_summary_length_ratio = 0
for i in range(len(train_chapters_length)):
    average_summary_length_ratio += train_summaries_length[i] / train_chapters_length[i]
average_summary_length_ratio /= len(train_chapters_length)


# In[11]:


import copy
import math
import numpy as np
import os
from sklearn.preprocessing import normalize

EPS = 0.000001


# In[12]:


from rouge_score import rouge_scorer
rougescorer = rouge_scorer.RougeScorer(['rouge1', 'rouge2', 'rougeL'], use_stemmer=True)


# In[13]:


# Train reference summaries
train_reference_summaries = train['summary_text'].tolist()
train_reference_summaries = [train_reference_summaries[i].replace("\n", " ") for i in range(len(train_reference_summaries))]


# In[20]:


# Train summary generation
train_pd = pd.read_csv("train_GAlabelled.csv", sep=";;;;;;", engine="python")
train_chapters_processed = max(train_pd['chapter'], default=0)

train_file = open("train_GAlabelled.csv", "a", encoding="utf-8")
#print("chapter;;;;;;sentence;;;;;;in_summary;;;;;;content", file = train_file)

train_generated_summaries = []
#for i in range(len(train_chapters)): # chapter i
for i in range(train_chapters_processed, len(train_chapters)):
    num_sentences = len(train_chapters_sentences[i])
    print("Train doc {}: {} sentences".format(i, len(train_chapters_sentences[i])))
    summary_length_limit = math.ceil(train_chapters_length[i] * average_summary_length_ratio)
    sentences_length = [len(word_tokenize(train_chapters_sentences[i][j])) for j in range(num_sentences)]
    #summary_length_limit = sum(sentences_length)

    inputFile = open("GA-ExtractiveOracle-input.txt", "w", encoding="utf-8")
    print("{} {}".format(num_sentences, summary_length_limit), file = inputFile)
    for j in range(num_sentences):
        print("{} ".format(sentences_length[j]), end = "", file = inputFile)
    print("", file = inputFile)
    print("{}".format(train_reference_summaries[i]), file = inputFile)
    for j in range(num_sentences):
        print("{}".format(train_chapters_sentences[i][j].replace('\ufffd', '')), file = inputFile)
    inputFile.close()

    os.system("GA-ExtractiveOracle.exe")
    chosen_sentences_index = []
    outputFile = open("GA-ExtractiveOracle-output.txt", "r")
    num_lines = int(outputFile.readline())
    for j in range(num_lines):
        temp_index = int(outputFile.readline())
        chosen_sentences_index.append(temp_index)
    current_profit = float(outputFile.readline())
    current_cost = int(outputFile.readline())
    print("Profit = {0:.9f}, ".format(current_profit), end="")
    print("cost = {} / {}".format(current_cost, summary_length_limit))
    outputFile.close()

    chosen_sentences_index.sort()
    chosen_sentences_sign = [0 for j in range(num_sentences)]
    generated_summary = ""
    for j in range(len(chosen_sentences_index)):
        chosen_sentences_sign[chosen_sentences_index[j]] = 1
        if j > 0:
            generated_summary += " "
        generated_summary += train_chapters_sentences[i][chosen_sentences_index[j]]
    for j in range(num_sentences):
        print("{};;;;;;{};;;;;;{};;;;;;{}".format(i + 1, j + 1, chosen_sentences_sign[j], train_chapters_sentences[i][j].replace('\ufffd', '')), file = train_file)
    train_generated_summaries.append(generated_summary)

train_file.close()

# ## End of [3]: Training

# ## Start of [4]: Validating

# In[21]:


# Validation reference summaries
val_reference_summaries = val['summary_text'].tolist()
val_reference_summaries = [val_reference_summaries[i].replace("\n", " ") for i in range(len(val_reference_summaries))]


# In[22]:


# Validation summary generation
val_pd = pd.read_csv("val_GAlabelled.csv", sep=";;;;;;", engine="python")
val_chapters_processed = max(val_pd['chapter'], default=0)

validation_file = open("val_GAlabelled.csv", "a", encoding="utf-8")
#print("chapter;;;;;;sentence;;;;;;in_summary;;;;;;content", file = validation_file)

val_generated_summaries = []
#for i in range(len(val_chapters)):
for i in range(val_chapters_processed, len(val_chapters)):
    num_sentences = len(val_chapters_sentences[i])
    print("Validation doc {}: {} sentences".format(i, len(val_chapters_sentences[i])))
    summary_length_limit = math.ceil(val_chapters_length[i] * average_summary_length_ratio)
    sentences_length = [len(word_tokenize(val_chapters_sentences[i][j])) for j in range(num_sentences)]
    #summary_length_limit = sum(sentences_length)

    inputFile = open("GA-ExtractiveOracle-input.txt", "w", encoding="utf-8")
    print("{} {}".format(num_sentences, summary_length_limit), file = inputFile)
    for j in range(num_sentences):
        print("{} ".format(sentences_length[j]), end = "", file = inputFile)
    print("", file = inputFile)
    print("{}".format(val_reference_summaries[i]), file = inputFile)
    for j in range(num_sentences):
        print("{}".format(val_chapters_sentences[i][j].replace('\ufffd', '')), file = inputFile)
    inputFile.close()

    os.system("GA-ExtractiveOracle.exe")
    chosen_sentences_index = []
    outputFile = open("GA-ExtractiveOracle-output.txt", "r")
    num_lines = int(outputFile.readline())
    for j in range(num_lines):
        temp_index = int(outputFile.readline())
        chosen_sentences_index.append(temp_index)
    current_profit = float(outputFile.readline())
    current_cost = int(outputFile.readline())
    print("Profit = {0:.9f}, ".format(current_profit), end="")
    print("cost = {} / {}".format(current_cost, summary_length_limit))
    outputFile.close()

    chosen_sentences_index.sort()
    chosen_sentences_sign = [0 for j in range(num_sentences)]
    generated_summary = ""
    for j in range(len(chosen_sentences_index)):
        chosen_sentences_sign[chosen_sentences_index[j]] = 1
        if j > 0:
            generated_summary += " "
        generated_summary += val_chapters_sentences[i][chosen_sentences_index[j]]
    for j in range(num_sentences):
        print("{};;;;;;{};;;;;;{};;;;;;{}".format(i + 1, j + 1, chosen_sentences_sign[j], val_chapters_sentences[i][j].replace('\ufffd', '')), file = validation_file)
    val_generated_summaries.append(generated_summary)

validation_file.close()

# ## End of [4]: Validating
