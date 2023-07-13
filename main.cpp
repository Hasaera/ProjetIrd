#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/http_listener.h>

#include <tensorflow/c/c_api.h>
#include <tensorflow/c/eager/c_api.h>

using namespace web::http;
using namespace web::http::experimental::listener;

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

// Chemin du fichier modèle .h5
const std::string model_path = "model.h5";

// Fonction d'inférence avec le modèle de neurones
std::string RunInference(const std::vector<float>& data)
{
    // Charger le modèle de neurones
    TF_Status* status = TF_NewStatus();
    TF_Graph* graph = TF_NewGraph();
    TF_SessionOptions* options = TF_NewSessionOptions();
    TF_Buffer* graph_def = nullptr;
    std::string error_msg;

    // Lire le fichier modèle .h5
    FILE* file = fopen(model_path.c_str(), "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        const size_t file_size = ftell(file);
        rewind(file);

        void* graph_data = malloc(file_size);
        fread(graph_data, 1, file_size, file);
        fclose(file);

        graph_def = TF_NewBuffer();
        graph_def->data = graph_data;
        graph_def->length = file_size;
        graph_def->data_deallocator = [](void* data, size_t length) {
            free(data);
        };
    }
    else
    {
        error_msg = "Failed to open model file.";
        return error_msg;
    }

    // Importer le graphique du modèle
    TF_ImportGraphDefOptions* import_options = TF_NewImportGraphDefOptions();
    TF_GraphImportGraphDef(graph, graph_def, import_options, status);
    TF_DeleteImportGraphDefOptions(import_options);
    TF_DeleteBuffer(graph_def);

    // Vérifier les erreurs lors de l'importation du graphique du modèle
    if (TF_GetCode(status) != TF_OK)
    {
        error_msg = "Failed to import model graph: ";
        error_msg += TF_Message(status);
        TF_DeleteStatus(status);
        return error_msg;
    }

    // Créer une session TensorFlow
    TF_Session* session = TF_NewSession(graph, options, status);
    TF_DeleteSessionOptions(options);
    if (TF_GetCode(status) != TF_OK)
    {
        error_msg = "Failed to create TensorFlow session: ";
        error_msg += TF_Message(status);
        TF_DeleteStatus(status);
        return error_msg;
    }

    // Créer le tenseur d'entrée avec les données
    const int batch_size = 1;  // Pour une seule instance de données
    const int num_features = data.size();
    const std::vector<std::int64_t> input_dims = { batch_size, num_features };
    std::vector<float> input_data(data.begin(), data.end());
    const size_t input_data_size = input_data.size() * sizeof(float);

    TF_Tensor* input_tensor = TF_AllocateTensor(TF_FLOAT, input_dims.data(), input_dims.size(), input_data_size);
    std::memcpy(TF_TensorData(input_tensor), input_data.data(), input_data_size);

    // Créer les entrées et sorties du modèle
    const std::vector<TF_Output> inputs = {
        { TF_GraphOperationByName(graph, "input_1"), 0 }
    };
    const std::vector<TF_Output> outputs = {
        { TF_GraphOperationByName(graph, "output_1"), 0 }
    };

    // Exécuter l'inférence avec le modèle
    std::vector<TF_Tensor*> input_tensors = { input_tensor };
    std::vector<TF_Tensor*> output_tensors(outputs.size(), nullptr);

    TF_SessionRun(session,
                  nullptr,
                  inputs.data(), input_tensors.data(), input_tensors.size(),
                  outputs.data(), output_tensors.data(), output_tensors.size(),
                  nullptr, 0, nullptr, status);

    // Vérifier les erreurs lors de l'exécution de l'inférence
    if (TF_GetCode(status) != TF_OK)
    {
        error_msg = "Failed to run model inference: ";
        error_msg += TF_Message(status);
        TF_DeleteStatus(status);
        TF_DeleteSession(session, status);
        TF_DeleteGraph(graph);
        return error_msg;
    }

    // Obtenir les résultats de l'inférence
    const std::vector<float> results(static_cast<float*>(TF_TensorData(output_tensors[0])),
                                     static_cast<float*>(TF_TensorData(output_tensors[0])) + TF_TensorElementCount(output_tensors[0]));

    // Convertir les résultats en chaîne JSON
    web::json::value json_results;
    for (size_t i = 0; i < results.size(); ++i)
    {
        json_results[U("output_") + utility::conversions::to_string_t(std::to_string(i))] = web::json::value::number(results[i]);
    }

    // Libérer les ressources TensorFlow
    TF_DeleteTensor(input_tensor);
    for (auto tensor : output_tensors)
    {
        TF_DeleteTensor(tensor);
    }
    TF_DeleteSession(session, status);
    TF_DeleteGraph(graph);
    TF_DeleteStatus(status);

    // Renvoyer les résultats au format JSON
    return json_results.serialize();
}

void HandlePostRequest(http_request request)
{
    concurrency::streams::istream bodyStream = request.body();
    std::vector<float> data;

    // Convert the vector of floats to a streambuf
    concurrency::streams::streambuf<unsigned char> bodyStreambuf;
    bodyStream.read_to_end(bodyStreambuf).wait();

    // Read the data from the streambuf into the vector of floats
    auto dataSize = bodyStreambuf.size();
    data.resize(dataSize / sizeof(float));
    bodyStreambuf.getn(reinterpret_cast<unsigned char*>(data.data()), dataSize).wait();

    // Perform inference with the neural network model
    std::string inferenceResult = RunInference(data);

    // Return the inference result as a response
    request.reply(status_codes::OK, inferenceResult, U("application/json"));
}


int main()
{
    // URL de l'API
    utility::string_t api_url = U("https://ird.com/api/neural-network");

    http_listener listener(api_url); 
  
    listener.support(methods::POST, HandlePostRequest);

    try
    {
        listener.open().wait();
        std::cout << "Listening for requests at: " << listener.uri().to_string() << std::endl;
        std::cout << "Press ENTER to exit." << std::endl;
        std::cin.ignore();
        listener.close().wait();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}


