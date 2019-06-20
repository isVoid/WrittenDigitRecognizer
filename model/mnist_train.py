import tensorflow as tf

#load mnist data
(x_train, y_train), (x_test, y_test) = tf.keras.datasets.mnist.load_data()
def create_mnist_dataset(data, labels, batch_size):
  def gen():
    for image, label in zip(data, labels):
        yield image, label
  ds = tf.data.Dataset.from_generator(gen, (tf.float32, tf.int32), ((28,28 ), ()))

  return ds.repeat().batch(batch_size)

#train and validation dataset with different batch size
train_dataset = create_mnist_dataset(x_train, y_train, 10)
valid_dataset = create_mnist_dataset(x_test, y_test, 20)

handle = tf.placeholder(tf.string, shape=[])
iterator = tf.data.Iterator.from_string_handle(
            handle, train_dataset.output_types, train_dataset.output_shapes)
            
image, label = iterator.get_next()
image = image / 255. #Normalize
image = tf.expand_dims(image, -1)

image = tf.identity(image, name="input_image")

train_iterator = train_dataset.make_one_shot_iterator()
valid_iterator = valid_dataset.make_one_shot_iterator()

conv1 = tf.layers.Conv2D(
    filters = 36,
    kernel_size = 14,
    padding = 'same',
    activation = tf.nn.relu,
    name = 'conv1'
)(image)

mp1 = tf.layers.MaxPooling2D(
    pool_size = 2,
    strides = 2,
    padding = 'same',
    name = 'maxpool1'
)(conv1)

conv2 = tf.layers.Conv2D(
    filters = 36,
    kernel_size = 7,
    padding = 'same',
    activation = tf.nn.relu,
    name = 'conv2'
)(mp1)

mp2 = tf.layers.MaxPooling2D(
    pool_size = 2,
    strides = 2,
    padding = 'same',
    name = 'maxpool1'
)(conv2)

conv3 = tf.layers.Conv2D(
    filters = 36,
    kernel_size = 4,
    padding = 'same',
    activation = tf.nn.relu,
    name = 'conv3'
)(mp2)

mp3 = tf.layers.MaxPooling2D(
    pool_size = 2,
    strides = 2,
    padding = 'same',
    name = 'maxpool3'
)(conv3)

flat = tf.layers.Flatten()(mp3)

dense1 = tf.layers.Dense(
    units = 576,
    activation = tf.nn.relu,
    name = "dense1"
)(flat)

dense1_dropout = tf.layers.Dropout(
    rate = 0.4,
    name = "dense1_dropout"
)(dense1)

dense2 = tf.layers.Dense(
    units = 10,
    activation = None,
    name = "dense2"
)(dense1_dropout)

loss = tf.losses.sparse_softmax_cross_entropy(labels=label, logits = dense2)

train_op = tf.train.AdamOptimizer(learning_rate=1e-4).minimize(loss)

accuracy = tf.metrics.accuracy(labels = label, predictions=tf.argmax(tf.nn.softmax(dense2), axis=1))

tf.Graph().finalize()

iters = 600000 #10epoch

saver = tf.train.Saver()

with tf.Session() as sess:
   tf.train.write_graph(sess.graph, '.', 'model/graph.pb', as_text=False)
    
   sess.run(tf.global_variables_initializer())
   sess.run(tf.local_variables_initializer())
   
   for i in range(iters):
       train_handle = sess.run(train_iterator.string_handle())
       _, train_loss = sess.run([train_op, loss], feed_dict={handle: train_handle})
       
       if i % 500 == 0:
           valid_handle = sess.run(valid_iterator.string_handle())
           acc = sess.run(accuracy, feed_dict={handle: valid_handle})
           
           print("iters: ", i,  " loss: ", train_loss, " accuracy: ", acc[1])
           saver.save(sess, 'model/mnist_', i)
                                                    
   
