namespace nen::mesh
{
    class ObjLoader
    {
    public:
        static bool Load(std::shared_ptr<class Renderer> renderer, std::string_view, std::string_view);
    };
}