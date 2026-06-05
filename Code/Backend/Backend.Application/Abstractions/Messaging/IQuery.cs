using MediatR;
using Backend.Domain.Kernel;

namespace Backend.Application.Abstractions.Messaging;

public interface IQuery<TResponse> : IRequest<Result<TResponse>>;
