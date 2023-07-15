# Maching Learning Notes 1


## 用pickle保存和加载模型
- 保存模型
    ```python
    import pickle
    from sklearn.svm import SVC
    model_dir = './model.pkl'
    model = SVC()
    with open(model_dir, 'wb') as f:
        pickle.dump(model, f)
        f.close() # 注意:保存完模型之后要关闭文件
    ```
- 加载模型
    ```python
    import pickle
    model_dir = './model.pkl'
    with open(model_dir, 'rb') as f:
        model = pickel.load(f)
    print(mode.predict(x))
    ```

## 逻辑回归 Logistic Regression
- LR Implementation code snippets
  ```python
    from sklearn.linear_model import LogisticRegression
    from sklearn.model_selection import train_test_split
    from sklearn.metrics import accuracy_score
    import numpy as np
    import matplotlib.pyplot as plt
    import pickle
    from tqdm import tqdm

    data_path = './data/merged_data/data.npy'
    data = np.load(data_path, allow_pickle=True)
    model_l1_path='./model/logistic_reg_l1.pickle'
    model_l2_path='./model/logictic_reg_l2.pickle'
    X = data[:,0:35]
    y = data[:, -1]

    X_train, x_test, Y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=1)

    # lr_l1 = LogisticRegression(penalty="l1", C=0.5, solver='sag', multi_class="auto")
    # lr_l2 = LogisticRegression(penalty="l2", C=0.5, solver='sag', multi_class="auto")

    # # train model
    # lr_l1.fit(X_train, Y_train)
    # lr_l2.fit(X_train, Y_train)

    # model performence on train set
    l1_train_predict = []
    l2_train_predict = []

    # model performence on test set
    l1_test_predict = []
    l2_test_predict = []

    for c in tqdm(np.linspace(0.01, 2, 50)):
        # lr_l1 = LogisticRegression(penalty="l1", C=c, solver='liblinear', max_iter=1000)
        # lr_l2 = LogisticRegression(penalty='l2', C=c, solver='liblinear', max_iter=1000)

        lr_l1 = LogisticRegression(penalty="l1", C=c, solver='liblinear', max_iter=1000, multi_class='auto')
        lr_l2 = LogisticRegression(penalty='l2', C=c, solver='liblinear', max_iter=1000, multi_class='auto')


        # 训练模型，记录L1正则化模型在训练集测试集上的表现
        lr_l1.fit(X_train, Y_train)
        l1_train_predict.append(accuracy_score(lr_l1.predict(X_train), Y_train))
        l1_test_predict.append(accuracy_score(lr_l1.predict(x_test), y_test))

        # 记录L2正则化模型的表现
        lr_l2.fit(X_train, Y_train)
        l2_train_predict.append(accuracy_score(lr_l2.predict(X_train), Y_train))
        l2_test_predict.append(accuracy_score(lr_l2.predict(x_test), y_test))

        if c == 2:
            pred_y_test = lr_l2.predict(x_test)
            mask = abs(pred_y_test-y_test) < 5
            neg_test = pred_y_test[mask]
            res = (len(neg_test)/len(pred_y_test))
            print(res)
            with open(model_l1_path, 'wb') as f1:
                pickle.dump(lr_l1, f1)
            with open(model_l2_path, 'wb') as f2:
                pickle.dump(lr_l2, f2)


    data = [l1_train_predict, l2_train_predict, l1_test_predict, l2_test_predict]
    label = ['l1_train', 'l2_train', 'l1_test', "l2_test"]
    color = ['red', 'green', 'orange', 'blue']

    plt.figure(figsize=(12, 6))
    for i in range(4) :
        plt.plot(np.linspace(0.01, 2, 50), data[i], label=label[i], color=color[i])

    plt.legend(loc="best")
    plt.show()
  ```


## 支持向量机 Support Vector Machine

- Using GridSearch to find the best parameters [code snippets]
  ```python
    import numpy as np
    import matplotlib.pyplot as plt
    from sklearn.linear_model import Perceptron, LogisticRegression
    from sklearn.svm import SVC
    from sklearn.model_selection import train_test_split
    from sklearn.preprocessing import StandardScaler
    from sklearn import datasets
    from sklearn import metrics
    import pickle
    merged_data_dir = '../data/merged_data/merged_data.npy'
    model_dir='./svm.pkl'
    data = np.load(merged_data_dir, allow_pickle=True)

    #labeling
    for ele in data:
        if ele[-1] < 20:
            ele[-1] = 0
        elif ele[-1] >=20 and ele[-1] < 40:
            ele[-1] = 1
        else:
            ele[-1] = 2


    X = data[:,0:34]
    y = data[:,-1]
    print(y)
    # Create training and test split
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=1, stratify=y)
    # feature scaling
    # sc = StandardScaler()
    # sc.fit(X_train)
    # X_train_std = sc.transform(X_train)
    # X_test_std = sc.transform(X_test)


    ##################################
    # # Instantiate the Support Vector Classifier (SVC)
    # svc = SVC(C=10, random_state=1, kernel='rbf', gamma=0.3)

    # # Fit the model
    # svc.fit(X_train, y_train)

    # # Make the predictions
    # y_predict = svc.predict(X_test)

    # # Measure the performance
    # print("Accuracy score %.3f" %metrics.accuracy_score(y_test, y_predict))
    #############################################

    def svm_cross_validation(train_x, train_y):
        from sklearn.model_selection import GridSearchCV
        from sklearn.svm import SVC
        model = SVC(kernel='rbf', probability=True)
        param_grid = {'C': [1e-3, 1e-2, 1e-1, 1, 10, 100, 1000], 'gamma': [0.001, 0.0001]}
        grid_search = GridSearchCV(model, param_grid, n_jobs = 8, verbose=1, scoring='accuracy')
        grid_search.fit(train_x, train_y)
        best_parameters = grid_search.best_estimator_.get_params()
        for para, val in list(best_parameters.items()):
            print(para, val)
        model = SVC(kernel='rbf', C=best_parameters['C'], gamma=best_parameters['gamma'], probability=True)
        model.fit(train_x, train_y)

        return model

    svm_model = svm_cross_validation(X_train, y_train)
    with open(model_dir, 'wb') as f1:
        pickle.dump(svm_model, f1)
        f1.close()
    print(svm_model.score(X_test, y_test))
    y_predict = svm_model.predict(X_test)
    print(y_predict)
  ```


---

> 作者: <no value>  
> URL: https://lruihao.cn/posts/notes_1/  

