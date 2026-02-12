namespace Backend.Application.Abstractions.Authentication
{
    public interface IUserContext
    {
        Guid GetUserId();
        UserMedatdata GetUserMedadata();
    }
}
