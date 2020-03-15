using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(Animator))]
public class BabaAuth : MonoBehaviour
{

    protected Animator AUTH_Baba_Animator;
    protected CharacterController AUTH_Baba_Controller;

    // Parameters
    readonly int Baba_HashGrounded = Animator.StringToHash("Grounded");

    // Status
    bool IsGrounded = false;

    // Value
    public float Gravity = 20f;
    public float verticalSpeed = 0.0f;
    public float stickingGravityProportion = 0.3f;
    public float jumpAbortSpeed = 10f;


    private void Awake()
    {
        AUTH_Baba_Animator = GetComponent<Animator>();
        AUTH_Baba_Controller = GetComponent<CharacterController>();
    }

    private void FixedUpdate()
    {
        if (AUTH_Baba_Controller.isGrounded)
            verticalSpeed = -Gravity * stickingGravityProportion;
        else
        {
            if (verticalSpeed > 0.0f)
                verticalSpeed -= jumpAbortSpeed * Time.deltaTime;
            if (Mathf.Approximately(verticalSpeed, 0f))
                verticalSpeed = 0f;

            verticalSpeed -= Gravity * Time.deltaTime;
        }
    }
    
    private void OnAnimatorMove()
    {
        Vector3 Movement;

        AUTH_Baba_Controller.transform.rotation = AUTH_Baba_Animator.deltaRotation;

        Movement = AUTH_Baba_Animator.deltaPosition;
        Movement += verticalSpeed * Vector3.up * Time.deltaTime;

        AUTH_Baba_Controller.Move(Movement);

        
     //   if (!AUTH_Baba_Controller.isGrounded)
      //      Debug.Log("IsGrounded : " + AUTH_Baba_Controller.isGrounded + "Position" + transform.position.y);
    }
}
